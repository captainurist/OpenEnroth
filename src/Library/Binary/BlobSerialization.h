#pragma once

#include <string>
#include <typeinfo>
#include <utility>

#include "Utility/Error/Result.h"
#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/Streams/BlobOutputStream.h"
#include "Utility/Memory/Blob.h"

#include "BinaryConcepts.h"
#include "BinaryExceptions.h"

template<class Src, class... Tags> requires (!starts_with_v<is_greedy_tag, Tags...>)
void serialize(const Src &src, Blob *dst, const Tags &... tags) {
    BlobOutputStream stream(dst);
    serialize(src, &stream, tags...);
    stream.close(); // Flush data into a Blob.
}

template<class Dst, class... Tags> requires (!starts_with_v<is_greedy_tag, Tags...>)
void deserialize(const Blob &src, Dst *dst, const Tags &... tags) {
    // Using MemoryInputStream and not BlobInputStream is intentional. BlobInputStream is heavier, and we don't need
    // its functionality here.
    MemoryInputStream stream(src.data(), src.size());
    deserialize(stream, dst, tags...);
    if (stream.position() != stream.size())
        throwBinarySerializationLeftoverDataError(stream.position(), stream.size() - stream.position(), typeid(Dst).name());
}

/**
 * Same as the `Blob`-based `deserialize`, but converts the exception it might throw into an `Error`.
 *
 * @param src                           Blob to deserialize from.
 * @param[out] dst                      Object to deserialize into. Left partially written on error.
 * @param tags                          Serialization tags.
 * @return                              Success, or the error that was encountered.
 * @see tryDeserialize(InputStream &, Dst *, const Tags &...)
 */
template<class Dst, class... Tags> requires (!starts_with_v<is_greedy_tag, Tags...>)
Result<void> tryDeserialize(const Blob &src, Dst *dst, const Tags &... tags) {
    return tryCatch([&] { deserialize(src, dst, tags...); });
}
