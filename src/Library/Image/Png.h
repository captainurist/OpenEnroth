#pragma once

#include "Library/Image/Image.h"
#include "Utility/Error/Result.h"
#include "Utility/Memory/Blob.h"

namespace png {

/**
 * @param data                          PNG image to decode.
 * @return                              Decoded `RgbaImage`, or an error if `data` is not a valid PNG image.
 */
[[nodiscard]] Result<RgbaImage> decode(const Blob &data);

/**
 * @param image                         Image to encode.
 * @return                              Encoded PNG data, or an error if libpng refused to encode the image.
 */
[[nodiscard]] Result<Blob> encode(RgbaImageView image);
[[nodiscard]] Result<Blob> encode(GrayscaleImageView image);

bool detect(const Blob &data);

} // namespace png
