#pragma once

#include <string>
#include <typeinfo>
#include <utility>

#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/Streams/BlobOutputStream.h"
#include "Utility/Memory/Blob.h"
#include "Utility/Exception.h"

#include "Utility/Error/Result.h"

#include "BinaryConcepts.h"
#include "BinaryErrors.h"

template<class Src, class... Tags> requires (!starts_with_v<is_greedy_tag, Tags...>)
void serialize(const Src &src, Blob *dst, const Tags &... tags) {
    BlobOutputStream stream(dst);
    serialize(src, &stream, tags...);
    stream.close(); // Flush data into a Blob.
}

/**
 * Deserializes an object from a `Blob`, requiring that the whole blob is consumed.
 *
 * @param src                           Blob to deserialize from.
 * @param[out] dst                      Object to deserialize into. Zero-filled on error.
 * @param tags                          Serialization tags.
 * @return                              Success, or the first error that was encountered.
 */
template<class Dst, class... Tags> requires (!starts_with_v<is_greedy_tag, Tags...>)
[[nodiscard]] Result<void> tryDeserialize(const Blob &src, Dst *dst, const Tags &... tags) {
    // Using MemoryInputStream and not BlobInputStream is intentional. BlobInputStream is heavier, and we don't need
    // its functionality here.
    MemoryInputStream stream(src.data(), src.size(), src.displayPath());
    deserialize(stream, dst, tags...);
    if (!stream.failed() && stream.position() != stream.size())
        setBinarySerializationLeftoverDataError(stream, stream.position(), stream.size() - stream.position(),
                                                typeid(Dst).name());
    return stream.check();
}

/**
 * Same as `tryDeserialize`, but throws on error.
 *
 * @deprecated This is a migration bridge for the code that hasn't been converted to `Result` yet. Use
 *             `tryDeserialize` in new code.
 *
 * @param src                           Blob to deserialize from.
 * @param[out] dst                      Object to deserialize into.
 * @param tags                          Serialization tags.
 * @throws Exception                    On error.
 */
template<class Dst, class... Tags> requires (!starts_with_v<is_greedy_tag, Tags...>)
void deserialize(const Blob &src, Dst *dst, const Tags &... tags) {
    Result<void> result = tryDeserialize(src, dst, tags...);
    if (!result)
        throw Exception("{}", result.error().message());
}
