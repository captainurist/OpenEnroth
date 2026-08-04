#pragma once

#include "CommonSerialization.h"
#include "BlobSerialization.h"
#include "BinaryConcepts.h"

namespace detail {
struct BlobSerializer {
    template<class T>
    Blob operator()(const T &value) const {
        Blob result;
        serialize(value, &result).mustSucceed(); // Writes to a Blob can't fail.
        return result;
    }
};

template<class T>
struct BlobDeserializer {
    T operator()(const Blob &blob) const {
        // TODO(captainurist): #exceptions This is a throwing bridge, port the callers to `Result`.
        T result;
        deserialize(blob, &result).orThrow();
        return result;
    }
};

template<class T>
struct StreamDeserializer {
    template<class... Tags>
    T operator()(InputStream &src, const Tags &... tags) const {
        // TODO(captainurist): #exceptions This is a throwing bridge, port the callers to `Result`.
        T val;
        deserialize(src, &val, tags...).orThrow();
        return val;
    }
};
} // namespace detail

/**
 * Ranges-friendly binary serialization object.
 *
 * Can be used directly as `return toBlob(someStruct)`, or passed into `std::views::transform`.
 *
 * Under the hood it's calling the `serialize` function using argument-dependent lookup.
 *
 * @see fromBlob
 */
constexpr detail::BlobSerializer toBlob;

/**
 * Ranges-friendly binary deserialization object.
 *
 * Can be used directly as `return fromBlob<T>(blob)`, or passed into `std::views::transform`.
 *
 * Under the hood it's calling the `deserialize` function using argument-dependent lookup.
 *
 * @see toBlob
 */
template<class T>
constexpr detail::BlobDeserializer<T> fromBlob;

/**
 * Value-returning binary stream deserialization object.
 *
 * Can be used as `fromStream<uint32_t>(stream)` or `fromStream<std::string>(stream, tags::nullTerminated)`.
 *
 * Under the hood it's calling the `deserialize` function using argument-dependent lookup.
 *
 * @see fromBlob
 */
template<class T>
constexpr detail::StreamDeserializer<T> fromStream;
