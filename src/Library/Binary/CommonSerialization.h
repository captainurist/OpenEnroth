#pragma once

#include <cassert>
#include <cstdint>
#include <span>
#include <array>
#include <typeinfo>
#include <string>

#include "BinaryFwd.h"
#include "BinaryTags.h"
#include "BinaryConcepts.h"
#include "BinaryErrors.h"

#include "Utility/Error/Result.h"
#include "Utility/Streams/InputStream.h"
#include "Utility/Streams/OutputStream.h"

namespace detail {
template<class Container>
struct AppendWrapper {
 public:
    using value_type = typename Container::value_type;

    explicit AppendWrapper(Container *container) : _container(container), _initialSize(container->size()) {}

    size_t size() const {
        return _container->size() - _initialSize;
    }

    auto data() const {
        return _container->data() + _initialSize;
    }

    void resize(size_t size) {
        _container->resize(_initialSize + size);
    }

 private:
    Container *_container = nullptr;
    size_t _initialSize = 0;
};
} // namespace detail


//
// Serialization for memcopy-able types.
//

template<class T> requires is_memcopy_serializable_v<T>
Result<void> serialize(const T &src, OutputStream *dst) {
    return dst->write(&src, sizeof(T));
}

template<class T> requires is_memcopy_serializable_v<T>
Result<void> deserialize(InputStream &src, T *dst) {
    Result<size_t> bytes = src.read(dst, sizeof(T));
    if (!bytes) [[unlikely]]
        return std::move(bytes).error();
    if (*bytes != sizeof(T)) [[unlikely]]
        return binarySerializationNoMoreDataError(*bytes, sizeof(T), typeid(T).name());
    return {};
}


//
// std::span support - doesn't write size to the stream.
//

// Note that the memcopy and per-element paths below are separate overloads and not an `if constexpr` on purpose.
// The looping paths propagate errors with explicit checks and stay plain functions - they run per element, and
// per-element code must never pay for a coroutine frame. See ExceptionFreeErrorHandling.md.
template<StdSpan Span, class T = typename Span::value_type> requires is_memcopy_serializable_v<T>
Result<void> deserialize(InputStream &src, Span *dst) {
    size_t bytesExpected = dst->size() * sizeof(T);
    Result<size_t> bytesRead = src.read(dst->data(), bytesExpected);
    if (!bytesRead) [[unlikely]]
        return std::move(bytesRead).error();
    if (*bytesRead != bytesExpected) [[unlikely]]
        return binarySerializationNoMoreDataError(*bytesRead % sizeof(T), sizeof(T), typeid(T).name());
    return {};
}

template<StdSpan Span, class T = typename Span::value_type> requires (!is_memcopy_serializable_v<T>)
Result<void> deserialize(InputStream &src, Span *dst) {
    for (T &element : *dst)
        if (Result<void> result = deserialize(src, &element); !result)
            return result;
    return {};
}

template<StdSpan Span, class T = typename Span::value_type> requires is_memcopy_serializable_v<T>
Result<void> serialize(const Span &src, OutputStream *dst) {
    return dst->write(src.data(), src.size() * sizeof(T));
}

template<StdSpan Span, class T = typename Span::value_type> requires (!is_memcopy_serializable_v<T>)
Result<void> serialize(const Span &src, OutputStream *dst) {
    for (const T &element : src)
        if (Result<void> result = serialize(element, dst); !result)
            return result;
    return {};
}


//
// std::span support with tag forwarding - doesn't write size to the stream.
//

template<StdSpan Span, class... Tags>
Result<void> deserialize(InputStream &src, Span *dst, EachTag, const Tags &... tags) {
    for (auto &element : *dst)
        if (Result<void> result = deserialize(src, &element, tags...); !result)
            return result;
    return {};
}

template<StdSpan Span, class... Tags>
Result<void> serialize(const Span &src, OutputStream *dst, EachTag, const Tags &... tags) {
    for (const auto &element : src)
        if (Result<void> result = serialize(element, dst, tags...); !result)
            return result;
    return {};
}


//
// std::array support - doesn't write size to the stream.
//

template<class T, size_t N, class... Tags>
Result<void> serialize(const std::array<T, N> &src, OutputStream *dst, const Tags &... tags) {
    return serialize(std::span(src), dst, tags...);
}

template<class T, size_t N, class... Tags>
Result<void> deserialize(InputStream &src, std::array<T, N> *dst, const Tags &... tags) {
    std::span span(*dst);
    return deserialize(src, &span, tags...);
}


//
// std::vector support - writes size to the stream, unless this is changed with tags.
//

template<ResizableContiguousContainer Src, class... Tags>
Result<void> serialize(const Src &src, OutputStream *dst, const Tags &... tags) {
    assert(src.size() <= UINT32_MAX);

    uint32_t size = src.size();
    if (Result<void> result = serialize(size, dst); !result)
        return result;
    std::span span(src.data(), src.size());
    return serialize(span, dst, tags...);
}

template<ResizableContiguousContainer Dst, class... Tags>
Result<void> deserialize(InputStream &src, Dst *dst, const Tags &... tags) {
    uint32_t size = 0;
    if (Result<void> result = deserialize(src, &size); !result)
        return result;

    // TODO(captainurist): can we do this better?
    // Best-effort check - number of records required can't be larger than the number of bytes in the stream.
    if (size > src.size() - src.position()) [[unlikely]]
        return binarySerializationNoMoreDataError(src.size() - src.position(), size, typeid(typename Dst::value_type).name());

    dst->resize(size);
    std::span span(dst->data(), dst->size());
    return deserialize(src, &span, tags...);
}

template<ResizableContiguousContainer Src, class... Tags>
Result<void> serialize(const Src &src, OutputStream *dst, UnsizedTag, const Tags &... tags) {
    return serialize(std::span(src), dst, tags...);
}

template<ResizableContiguousContainer Dst, class... Tags>
Result<void> deserialize(InputStream &src, Dst *dst, PresizedTag tag, const Tags &... tags) {
    dst->resize(tag.size);
    std::span span(dst->data(), dst->size());
    return deserialize(src, &span, tags...);
}

template<ResizableContiguousContainer Dst, class... Tags>
Result<void> deserialize(InputStream &src, Dst *dst, AppendTag, const Tags &... tags) {
    detail::AppendWrapper wrapper(dst);
    return deserialize(src, &wrapper, tags...);
}


//
// Serialization for null-terminated strings.
//

inline Result<void> deserialize(InputStream &src, std::string *dst, NullTerminatedTag) {
    if (Result<size_t> bytesRead = src.readUntil('\0', dst); !bytesRead)
        return std::move(bytesRead).error();
    return {};
}

