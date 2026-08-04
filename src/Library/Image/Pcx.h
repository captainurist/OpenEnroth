#pragma once

#include "Library/Image/Image.h"
#include "Utility/Error/Result.h"
#include "Utility/Memory/Blob.h"

namespace pcx {
/**
 * Decodes a PCX image from a `Blob`.
 *
 * @param data                          Compressed PCX image to decode.
 * @return                              Decoded `RgbaImage`, or an error if `data` is not a valid PCX image.
 */
[[nodiscard]] Result<RgbaImage> decode(const Blob &data);

Blob encode(RgbaImageView image);

/**
 * @param data                          Compressed PCX image data.
 * @return                              Whether the data signature matches the PCX format.
 */
bool detect(const Blob &data);
}  // namespace pcx
