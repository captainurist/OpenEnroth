#pragma once

#include "LodFont.h"
#include "Library/Image/Palette.h"
#include "Library/Geometry/Size.h"
#include "Utility/Error/Result.h"

#include "LodImage.h"
#include "LodSprite.h"
#include "LodFont.h"

class Blob;

namespace lod {

bool detectCompressedData(const Blob &blob);
bool detectCompressedPseudoImage(const Blob &blob);
bool detectImage(const Blob &blob, bool *isPalette = nullptr);
bool detectSprite(const Blob &blob);
bool detectFont(const Blob &blob);

/**
 * This function processes compressed lod data.
 *
 * @param blob                          `Blob` from a LOD file.
 * @return                              Uncompressed `Blob`, or an error if the format is not recognized.
 */
[[nodiscard]] Result<Blob> decodeCompressedData(const Blob &blob);

/**
 * This function processes compressed lod pseudo-images.
 *
 * @param blob                          `Blob` from a LOD file.
 * @return                              Uncompressed `Blob`, or an error if the format is not recognized.
 */
[[nodiscard]] Result<Blob> decodeCompressedPseudoImage(const Blob &blob);

/**
 * This functions processes compressed lod data, and compressed lod pseudo-images.
 *
 * For everything else it just does nothing and returns the blob as is.
 *
 * @param blob                          `Blob` from a LOD file.
 * @return                              Uncompressed `Blob`, or an error if the format is not recognized.
 */
[[nodiscard]] Result<Blob> decodeMaybeCompressed(const Blob &blob);

/**
 * This function compresses the provided `Blob` into the compressed lod data format.
 *
 * @param blob                          `Blob` to compress.
 * @return                              Compressed `Blob`.
 */
Blob encodeCompressed(const Blob &blob);

/**
 * This function processes lod images and lod palettes. In case of the former, the pixel data is ignored.
 *
 * @param blob                          `Blob` from a LOD file.
 * @return                              Decoded `Palette`, or an error if the format is not recognized.
 */
[[nodiscard]] Result<Palette> decodePalette(const Blob &blob);

/**
 * This function processes lod images and lod palettes. In case of the latter, the pixel data will be empty.
 *
 * @param blob                          Image `Blob`, as read from a LOD file.
 * @return                              Decoded `LodImage`, or an error if the format is not recognized.
 */
[[nodiscard]] Result<LodImage> decodeImage(const Blob &blob);

/**
 * This function processes lod images and lod palettes. It reads the image header and returns image size w/o
 * decompressing the pixel data. For lod palettes returned image size will be zero.
 *
 * @param blob                          Image `Blob`, as read from a LOD file.
 * @return                              Image size, or an error if the format is not recognized.
 */
[[nodiscard]] Result<Sizei> decodeImageSize(const Blob &blob);

/**
 * This function processes lod sprites.
 *
 * @param blob                          Sprite `blob`, as read from a LOD file.
 * @return                              Decoded `LodSprite`, or an error if the format is not recognized.
 */
[[nodiscard]] Result<LodSprite> decodeSprite(const Blob &blob);

/**
 * This function processes lod fonts.
 *
 * @param blob                          Font `blob`, as read from a LOD file.
 * @return                              Decoded `LodFont`, or an error if the format is not recognized.
 */
[[nodiscard]] Result<LodFont> decodeFont(const Blob &blob);

} // namespace lod
