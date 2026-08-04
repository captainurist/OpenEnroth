#include "Png.h"

#include <memory>
#include <string>
#include <utility>

#define PNG_SIMPLIFIED_READ_SUPPORTED
#define PNG_SIMPLIFIED_WRITE_SUPPORTED
#include <png.h> // NOLINT: not a C system header.

#include "Utility/Error/Result.h"

Result<RgbaImage> png::decode(const Blob &data) {
    png_image pngImage = {};
    pngImage.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_memory(&pngImage, data.data(), data.size()))
        return fail("Failed to read PNG image '{}' ({}).", data.displayPath(), pngImage.message);

    pngImage.format = PNG_FORMAT_RGBA; // Format we want.

    RgbaImage result = RgbaImage::uninitialized(pngImage.width, pngImage.height);
    if (!png_image_finish_read(&pngImage, nullptr, result.pixels().data(), 0, nullptr)) {
        std::string message = pngImage.message;
        png_image_free(&pngImage);
        return fail("Failed to read PNG image '{}' ({}).", data.displayPath(), message);
    }

    png_image_free(&pngImage);
    return result;
}

template<class Color>
static Result<Blob> encodeWithFormat(ImageView<Color> image, int format) {
    png_image pngImage = {};
    pngImage.version = PNG_IMAGE_VERSION;
    pngImage.width = image.width();
    pngImage.height = image.height();
    pngImage.format = format;

    size_t size = PNG_IMAGE_PNG_SIZE_MAX(pngImage);
    std::unique_ptr<void, FreeDeleter> data(malloc(size));
    if (!png_image_write_to_memory(&pngImage, data.get(), &size, 0, image.pixels().data(), 0, nullptr))
        return fail("Failed to encode PNG image ({})", pngImage.message);

    return Blob::fromMalloc(std::move(data), size);
}

Result<Blob> png::encode(RgbaImageView image) {
    return encodeWithFormat(image, PNG_FORMAT_RGBA);
}

Result<Blob> png::encode(GrayscaleImageView image) {
    return encodeWithFormat(image, PNG_FORMAT_GRAY);
}

bool png::detect(const Blob &data) {
    // PNG files start with an 8-byte signature.
    if (data.size() < 8)
        return false;

    // Forward to libpng’s signature checker.
    return png_sig_cmp(static_cast<png_const_bytep>(data.data()), 0, 8) == 0;
}
