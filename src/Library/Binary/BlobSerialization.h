#pragma once

#include <string>
#include <typeinfo>
#include <utility>

#include "Utility/Error/Result.h"
#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/Streams/BlobOutputStream.h"
#include "Utility/Memory/Blob.h"

#include "BinaryConcepts.h"
#include "BinaryErrors.h"

template<class Src, class... Tags> requires (!starts_with_v<is_greedy_tag, Tags...>)
Result<void> serialize(const Src &src, Blob *dst, const Tags &... tags) {
    BlobOutputStream stream(dst);
    if (Result<void> result = serialize(src, &stream, tags...); !result)
        return result;
    return stream.close(); // Flush data into a Blob.
}

template<class Dst, class... Tags> requires (!starts_with_v<is_greedy_tag, Tags...>)
Result<void> deserialize(const Blob &src, Dst *dst, const Tags &... tags) {
    // Using MemoryInputStream and not BlobInputStream is intentional. BlobInputStream is heavier, and we don't need
    // its functionality here.
    MemoryInputStream stream(src.data(), src.size());

    Result<void> result = deserialize(stream, dst, tags...);
    if (result && stream.position() != stream.size()) [[unlikely]]
        result = binarySerializationLeftoverDataError(stream.position(), stream.size() - stream.position(), typeid(Dst).name());
    if (!result && !src.displayPath().empty()) [[unlikely]]
        return std::move(result).withContext("While reading '{}'", src.displayPath());
    return result;
}

