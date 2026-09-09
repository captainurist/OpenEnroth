#include "ResourceManager.h"

#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/TileGenerator.h"
#include "Engine/Data/ResourceMask.h"

#include "Library/Serialization/SerializationExceptions.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/FileSystem/Interface/FileSystem.h"
#include "Library/Json/Json.h"
#include "Library/Image/ImageFunctions.h"
#include "Library/Image/Pcx.h"
#include "Library/Image/Png.h"
#include "Library/Logger/Logger.h"
#include "Library/Magic/Magic.h"

#include "Utility/String/Ascii.h"
#include "Utility/ArrayAccess.h"
#include "Utility/Exception.h"
#include "Utility/MapAccess.h"

#include "EngineFileSystem.h"

ResourceManager::ResourceManager() = default;
ResourceManager::~ResourceManager() = default;

void ResourceManager::open() {
    from_json(Json::parse(dfs->read("data/resource_mask_table.json").string_view()), _masks);

    _eventsLodReader.open(dfs->read("data/events.lod"));
    _bitmapsLodReader.open(dfs->read("data/bitmaps.lod"));
    _iconsLodReader.open(dfs->read("data/icons.lod"));
    _spriteLodReader.open(dfs->read("data/sprites.lod"));

#if 0
    for (const std::string &entry : _bitmapsLodReader.ls()) {
        //if (!_masks->icons.contains(entry))
        //    continue;

        Blob data = lod::decodeMaybeCompressed(_bitmapsLodReader.read(entry));
        MagicFileFormat format = magic(data);
        if (format == MAGIC_LOD_IMAGE) {
            LodImage lodImage = lod::decodeImage(data);

            ResourceMask mask = valueOr(_masks.bitmaps, entry, ResourceMask());

            if (mask.mode == MASK_COLOR) {
                if (lodImage.palette.colors[0] != mask.color) {
                    fmt::println("BUGGY TRR {} {} {}", entry, toString(lodImage.palette.colors[0]), toString(mask));
                } else if (lodImage.zeroIsTransparent) {
                    fmt::println("GOOOD TRR {} {} {}", entry, toString(lodImage.palette.colors[0]), toString(mask));
                } else {
                    fmt::println("BUGGY ZZZ {} {} {}", entry, toString(lodImage.palette.colors[0]), toString(mask));
                }
            } else if (mask.mode == MASK_DEFAULT) {
                if (lodImage.zeroIsTransparent) {
                    fmt::println("BUGGY ZZZ {} {} {}", entry, toString(lodImage.palette.colors[0]), toString(mask));
                } else {
                    fmt::println("GOOOD OOO {} {} {}", entry, toString(lodImage.palette.colors[0]), toString(mask));
                }
            }
        }
    }
#endif

    // TODO(captainurist):
    //  on exception:
    //      Error(localization->str(LSTR_MIGHT_AND_MAGIC_VII_IS_HAVING_TROUBLE), localization->str(LSTR_REINSTALL_NECESSARY));
    // but we can't use localization object here cause it's not yet initialized.
}

Blob ResourceManager::event(std::string_view filename) {
    if (!_eventsLodReader.exists(filename)) {
        logger->error("Trying to load non-existent LOD entry '{}'.", _eventsLodReader.displayPath(filename));
        return {};
    }

    return lod::decodeMaybeCompressed(_eventsLodReader.read(filename));
}

RgbaImage ResourceManager::bitmap(std::string_view filename) {
    if (!_bitmapsLodReader.exists(filename)) {
        logger->error("Trying to load non-existent LOD entry '{}'.", _bitmapsLodReader.displayPath(filename));
        return {};
    }

    LodImage lodImage = lod::decodeImage(_bitmapsLodReader.read(filename));
    pPaletteManager->desaturate(&lodImage.palette); // Desaturate bitmaps.

    int maskIndex = -1;
    applyMask(valueOr(_masks.bitmaps, ascii::toLower(filename)), &lodImage, &maskIndex);

    // We use bilinear filtering for bitmaps, and to prevent bleeding of the mask color through the borders
    // we need to adjust the color of masked border pixels.
    return makeRgbaImageWithInterpolatedMask(lodImage.image, lodImage.palette, maskIndex);
}

RgbaImage ResourceManager::generated(std::string_view filename) {
    if (!pTileGenerator->hasTile(filename)) {
        logger->error("Trying to load non-existent generated tile '{}'.", ufs->displayPath(filename));
        return {};
    }

    pTileGenerator->ensureTile(filename);
    RgbaImage result = png::decode(ufs->read(filename));
    pPaletteManager->desaturate(&result); // Desaturate bitmaps.
    return result;
}

RgbaImage ResourceManager::icon(std::string_view filename) {
    if (!_iconsLodReader.exists(filename)) {
        logger->error("Trying to load non-existent LOD entry '{}'.", _iconsLodReader.displayPath(filename));
        return {};
    }

    Blob data = lod::decodeMaybeCompressed(_iconsLodReader.read(filename));
    MagicFileFormat format = magic(data);
    if (format == MAGIC_PCX) {
        RgbaImage image = pcx::decode(data);
        applyMask(valueOr(_masks.icons, ascii::toLower(filename)), &image);
        return image;
    } else if (format == MAGIC_LOD_IMAGE) {
        LodImage lodImage = lod::decodeImage(data);
        applyMask(valueOr(_masks.icons, ascii::toLower(filename)), &lodImage);
        return makeRgbaImage(lodImage.image, lodImage.palette);
    } else {
        throw Exception("File '{}' is not an image (recognized as {})", data.displayPath(), formatDescription(format));
    }
}

RgbaImage ResourceManager::sprite(std::string_view filename) {
    if (!_spriteLodReader.exists(filename)) {
        logger->error("Trying to load non-existent LOD entry '{}'.", _spriteLodReader.displayPath(filename));
        return {};
    }

    LodSprite lodSprite = lod::decodeSprite(_spriteLodReader.read(filename));

    RgbaImage result = RgbaImage::solid(lodSprite.image.width(), lodSprite.image.height(), Color());
    for (size_t y = 0, maxy = lodSprite.image.height(); y < maxy; y++) {
        for (size_t x = 0, maxx = lodSprite.image.width(); x < maxx; x++) {
            uint8_t pixel = lodSprite.image[y][x];
            result[y][x] = Color(pixel, 0, 0, pixel == 0 ? 0 : 255);
        }
    }
    return result;
}

RgbaImage ResourceManager::makeRgbaImageWithInterpolatedMask(GrayscaleImageView indexedImage, const Palette &palette, int maskIndex) {
    auto processTransparentPixel = [](GrayscaleImageView image, const Palette &palette, int x, int y, int transparent) {
        int count = 0;
        int r = 0, g = 0, b = 0;

        auto processPixel = [&](int x, int y) {
            uint8_t pixel = image[y][x];
            if (pixel != transparent) {
                count++;
                r += palette.colors[pixel].r;
                g += palette.colors[pixel].g;
                b += palette.colors[pixel].b;
            }
        };

        bool canDecX = x > 0;
        bool canIncX = x < image.width() - 1;
        bool canDecY = y > 0;
        bool canIncY = y < image.height() - 1;

        if (canDecX && canDecY)
            processPixel(x - 1, y - 1);
        if (canDecX)
            processPixel(x - 1, y);
        if (canDecX && canIncY)
            processPixel(x - 1, y + 1);
        if (canDecY)
            processPixel(x, y - 1);
        if (canIncY)
            processPixel(x, y + 1);
        if (canIncX && canDecY)
            processPixel(x + 1, y - 1);
        if (canIncX)
            processPixel(x + 1, y);
        if (canIncX && canIncY)
            processPixel(x + 1, y + 1);

        if (count != 0) {
            r /= count;
            g /= count;
            b /= count;
        }

        return Color(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), 0);
    };

    RgbaImage result = RgbaImage::uninitialized(indexedImage.width(), indexedImage.height());
    for (int y = 0, maxy = indexedImage.height(); y < maxy; y++) {
        for (int x = 0, maxx = indexedImage.width(); x < maxx; x++) {
            uint8_t pixel = indexedImage[y][x];
            if (pixel == maskIndex) {
                result[y][x] = processTransparentPixel(indexedImage, palette, x, y, maskIndex);
            } else {
                result[y][x] = palette.colors[pixel];
            }
        }
    }
    return result;
}

int ResourceManager::maskIndexFor(const LodImage &image, const ResourceMask &mask) {
    switch (mask.mode) {
    default: assert(false); [[fallthrough]];
    case MASK_DEFAULT:  return image.zeroIsTransparent ? 0 : -1;
    case MASK_NONE:     return -1;
    case MASK_ZERO:     return 0;
    case MASK_COLOR:    return indexOf(image.palette.colors, mask.color);
    }
}

void ResourceManager::applyMask(const ResourceMask &mask, LodImage *image, int *maskIndex) {
    int index = maskIndexFor(*image, mask);
    if (index != -1)
        image->palette.colors[index] = Color(0, 0, 0, 0);
    if (maskIndex)
        *maskIndex = index;
}

void ResourceManager::applyMask(const ResourceMask &mask, RgbaImage *image) {
    if (mask.mode != MASK_COLOR)
        return;

    for (size_t y = 0, maxy = image->height(); y < maxy; y++)
        for (size_t x = 0, maxx = image->width(); x < maxx; x++)
            if ((*image)[y][x] == mask.color)
                (*image)[y][x] = Color();
}
