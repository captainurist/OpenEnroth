#include "LodFormats.h"

#include <optional>
#include <span>
#include <vector>
#include <string>
#include <algorithm>
#include <ranges>

#include "LodFormatSnapshots.h"

#include "Library/Binary/CommonSerialization.h"
#include "Library/Snapshots/CommonSnapshots.h"
#include "Library/Compression/Compression.h"
#include "Library/Serialization/EnumSerialization.h"
#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/Streams/BlobInputStream.h"
#include "Utility/Memory/Blob.h"
#include "Utility/Error/Result.h"
#include "Utility/Lambda.h"

enum {
    MIN_GLYPH_WIDTH = 1,
    MAX_GLYPH_WIDTH = 63,
    MIN_GLYPH_HEIGHT = 4,
    MAX_GLYPH_HEIGHT = 63,
    MAX_GLYPH_SPACING = 63,
};

static Result<void> deserialize(InputStream &src, Palette *dst) {
    std::array<std::uint8_t, 0x300> rawPalette;
    src.readOrFail(rawPalette.data(), rawPalette.size()); // TODO(captainurist): #exceptions still throws.

    for (size_t i = 0; i < 256; i++)
        dst->colors[i] = Color(rawPalette[i * 3], rawPalette[i * 3 + 1], rawPalette[i * 3 + 2]);
    return {};
}

bool lod::detectCompressedData(const Blob &blob) {
    if (blob.size() < sizeof(LodCompressionHeader_MM6))
        return false;

    MemoryInputStream stream(blob.data(), blob.size());
    LodCompressionHeader_MM6 header;
    if (!deserialize(stream, &header))
        return false;

    return header.version == 91969 && memcmp(header.signature.data(), "mvii", 4) == 0;
}

bool lod::detectCompressedPseudoImage(const Blob &blob) {
    if (blob.size() < sizeof(LodImageHeader_MM6))
        return false;

    MemoryInputStream stream(blob.data(), blob.size());
    LodImageHeader_MM6 header;
    if (!deserialize(stream, &header))
        return false;

    return header.size == 0 && header.dataSize > 0 && header.width == 0 && header.height == 0 &&
        header.widthLn2 == 0 && header.heightLn2 == 0 &&
        header.paletteId == 0 && header.anotherPaletteId == 0 && (header.flags & 256) &&
        blob.size() == sizeof(LodImageHeader_MM6) + header.dataSize;
}

bool lod::detectImage(const Blob &blob, bool *isPalette) {
    if (isPalette)
        *isPalette = false;

    if (blob.size() < sizeof(LodImageHeader_MM6))
        return false;

    MemoryInputStream stream(blob.data(), blob.size());
    LodImageHeader_MM6 header;
    if (!deserialize(stream, &header))
        return false;

    if (header.size == 0 && header.dataSize == 0 && header.width == 0 && header.height == 0 &&
        header.widthLn2 == 0 && header.heightLn2 == 0 && header.widthMinus1 == 0 && header.heightMinus1 == 0 &&
        header.paletteId == 0 && header.anotherPaletteId == 0 && header.decompressedSize == 0 && header.flags == 0 &&
        blob.size() == sizeof(LodImageHeader_MM6) + 0x300)
        return isPalette ? *isPalette = true : true;

    if (header.size > 0 && header.dataSize > 0 && header.width > 0 && header.height > 0 &&
        header.size == header.width * header.height &&
        (header.decompressedSize == 0 && header.dataSize >= header.size || header.decompressedSize > 0 && header.decompressedSize >= header.size) &&
        blob.size() == sizeof(LodImageHeader_MM6) + header.dataSize + 0x300)
        return true;

    return false;
}

bool lod::detectSprite(const Blob &blob) {
    if (blob.size() < sizeof(LodSpriteHeader_MM6))
        return false;

    MemoryInputStream stream(blob.data(), blob.size());
    LodSpriteHeader_MM6 header;
    if (!deserialize(stream, &header))
        return false;

    if (header.dataSize > 0 && header.width > 0 && header.height > 0 &&
        header.paletteId > 0 && header.unk_0 == 0 && header.emptyBottomLines <= header.height &&
        blob.size() == sizeof(LodSpriteHeader_MM6) + header.height * sizeof(LodSpriteLine_MM6) + header.dataSize)
        return true;

    return false;
}

bool lod::detectFont(const Blob &blob) {
    if (blob.size() < sizeof(LodFontHeader_MM7) + std::min(sizeof(LodFontAtlas_MM7), sizeof(LodFontAtlas_MMX)))
        return {};

    MemoryInputStream stream(blob.data(), blob.size());
    LodFontHeader_MM7 header;
    if (!deserialize(stream, &header))
        return false;

    if (header.firstChar < header.lastChar && header.field_3 == 8 && header.field_4 == 0 && header.field_5 == 0 &&
        header.height >= MIN_GLYPH_HEIGHT && header.height <= MAX_GLYPH_HEIGHT && header.field_7 == 0 &&
        header.field_8 == 0 && header.paletteCount == 0 &&
        std::ranges::all_of(header.palettes, _1 == 0))
        return true;

    return false;
}

Result<Blob> lod::decodeCompressedData(const Blob &blob) {
    if (!detectCompressedData(blob))
        co_return fail("Cannot decode LOD entry '{}' as LOD compressed data", blob.displayPath());

    BlobInputStream stream(blob);
    LodCompressionHeader_MM6 header;
    co_await deserialize(stream, &header);

    Blob result;
    if (header.dataSize == blob.size()) {
        // Workaround for a bug in the original LOD writer, where header.dataSize was equal to LOD record size,
        // instead of the size of the data that followed.
        result = stream.readAllAsBlob();
    } else {
        result = stream.readAsBlobOrFail(header.dataSize); // TODO(captainurist): #exceptions still throws, caught by the coroutine.
    }
    if (header.decompressedSize)
        result = co_await zlib::uncompress(result.withDisplayPath(blob.displayPath()), header.decompressedSize);
    co_return result.withDisplayPath(blob.displayPath());
}

Result<Blob> lod::decodeCompressedPseudoImage(const Blob &blob) {
    if (!detectCompressedPseudoImage(blob))
        co_return fail("Cannot decode LOD entry '{}' as LOD compressed pseudo image", blob.displayPath());

    BlobInputStream stream(blob);
    LodImageHeader_MM6 header;
    co_await deserialize(stream, &header);

    Blob result = stream.readAsBlobOrFail(header.dataSize); // TODO(captainurist): #exceptions still throws, caught by the coroutine.
    if (header.decompressedSize)
        result = co_await zlib::uncompress(result.withDisplayPath(blob.displayPath()), header.decompressedSize);
    co_return result.withDisplayPath(blob.displayPath());
}

Result<Blob> lod::decodeMaybeCompressed(const Blob &blob) {
    if (detectCompressedData(blob))
        return decodeCompressedData(blob);

    if (detectCompressedPseudoImage(blob))
        return decodeCompressedPseudoImage(blob);

    return Blob::share(blob); // Not compressed.
}

Blob lod::encodeCompressed(const Blob &blob) {
    Blob compressed = zlib::compress(blob);

    LodCompressionHeader_MM6 header;
    header.version = 91969;
    header.signature = {{'m', 'v', 'i', 'i'}};
    header.dataSize = compressed.size();
    header.decompressedSize = blob.size();

    return Blob::concat(Blob::view(&header, sizeof(header)), compressed);
}

Result<Palette> lod::decodePalette(const Blob &blob) {
    if (!detectImage(blob))
        co_return fail("Cannot decode LOD entry '{}' as LOD palette", blob.displayPath());

    MemoryInputStream stream(blob.data(), blob.size(), blob.displayPath());
    LodImageHeader_MM6 header;
    co_await deserialize(stream, &header);

    stream.skipOrFail(header.dataSize); // TODO(captainurist): #exceptions still throws, caught by the coroutine.

    Palette result;
    co_await deserialize(stream, &result);
    co_return result;
}

Result<LodImage> lod::decodeImage(const Blob &blob) {
    bool isPalette = false;
    if (!detectImage(blob, &isPalette))
        co_return fail("Cannot decode LOD entry '{}' as LOD image", blob.displayPath());

    BlobInputStream stream(blob);
    LodImageHeader_MM6 header;
    co_await deserialize(stream, &header);

    Blob pixels;
    if (!isPalette) {
        pixels = stream.readAsBlobOrFail(header.dataSize); // TODO(captainurist): #exceptions still throws, caught by the coroutine.
        if (header.decompressedSize)
            pixels = co_await zlib::uncompress(pixels.withDisplayPath(blob.displayPath()), header.decompressedSize);

        // Note that this check isn't redundant. The checks in magic() only check sizes as written in the header.
        // Actual stream size might be different.
        if (pixels.size() < header.width * header.height)
            co_return fail("Cannot decode image LOD entry '{}': expected {}x{}={} pixels, got {}",
                           blob.displayPath(), header.width, header.height,
                           header.width * header.height, pixels.size());
    }

    LodImage result;
    co_await deserialize(stream, &result.palette);
    result.zeroIsTransparent = header.flags & 512;

    // TODO(captainurist): just store blob in GrayscaleImage, no need to copy here.
    if (pixels)
        result.image = GrayscaleImage::copy(static_cast<const uint8_t *>(pixels.data()), header.width, header.height); // NOLINT: this is not std::copy.
    co_return result;
}

Result<Sizei> lod::decodeImageSize(const Blob &blob) {
    if (!detectImage(blob))
        co_return fail("Cannot decode LOD entry '{}' as LOD image", blob.displayPath());

    BlobInputStream stream(blob);
    LodImageHeader_MM6 header;
    co_await deserialize(stream, &header);

    co_return Sizei(header.width, header.height);
}

Result<LodSprite> lod::decodeSprite(const Blob &blob) {
    if (!detectSprite(blob))
        co_return fail("Cannot decode LOD entry '{}' as LOD sprite", blob.displayPath());

    BlobInputStream stream(blob);
    LodSpriteHeader_MM6 header;
    co_await deserialize(stream, &header);

    std::vector<LodSpriteLine_MM6> lines;
    co_await deserialize(stream, &lines, tags::presized(header.height));

    Blob pixels = stream.readAsBlobOrFail(header.dataSize); // TODO(captainurist): #exceptions still throws, caught by the coroutine.
    if (header.decompressedSize)
        pixels = co_await zlib::uncompress(pixels.withDisplayPath(blob.displayPath()), header.decompressedSize);

    LodSprite result;
    result.paletteId = header.paletteId;
    result.image = GrayscaleImage::solid(0, header.width, header.height);

    for (size_t y = 0; y < header.height; y++) {
        const LodSpriteLine_MM6 &line = lines[y];

        if (line.begin == line.end)
            continue; // Empty line.

        if (line.begin < 0 || line.end < 0 || line.begin > header.width || line.end > header.width || line.begin > line.end ||
            line.offset > pixels.size() || line.offset + line.end - line.begin > pixels.size())
            co_return fail("Cannot decode sprite LOD entry '{}': invalid sprite line encountered at y={}",
                           blob.displayPath(), y);

        memcpy(result.image[y].data() + line.begin, static_cast<const char *>(pixels.data()) + line.offset, line.end - line.begin);
    }

    co_return result;
}

Result<LodFont> lod::decodeFont(const Blob &blob) {
    if (!detectFont(blob))
        return fail("Cannot decode LOD entry '{}' as LOD font", blob.displayPath());

    auto fixAndValidateFont = [](const Blob &blob, LodFont &font) -> Result<void> {
        for (int c = 0; c <= 255; c++) {
            if (c < font._header.firstChar || c > font._header.lastChar) {
                font._atlas.metrics[c].width = 0;
                font._atlas.metrics[c].leftSpacing = 0;
                font._atlas.metrics[c].rightSpacing = 0;
                font._atlas.offsets[c] = 0;
                continue;
            }

            // Check that font metrics are sane.
            const LodFontMetrics &metrics = font._atlas.metrics[c];
            if (metrics.width < MIN_GLYPH_WIDTH || metrics.width > MAX_GLYPH_WIDTH || metrics.leftSpacing > MAX_GLYPH_SPACING || metrics.rightSpacing > MAX_GLYPH_SPACING)
                return fail("Cannot decode font LOD entry '{}': invalid font metrics encountered for character #{}",
                            blob.displayPath(), c);

            // Check that all offsets point into the pixel data.
            int offset = font._atlas.offsets[c];
            int size = font._header.fontHeight * font._atlas.metrics[c].width;
            if (offset < 0 || size < 0 || size + offset > font._pixels.size())
                return fail("Cannot decode font LOD entry '{}': invalid glyph data encountered for character #{}",
                            blob.displayPath(), c);
        }
        return {};
    };

    // The font atlas comes in two flavors, and the only way to tell them apart is to try both. Note that this used
    // to be done with a try/catch – with `Result` it's just an `if`.
    auto tryDecode = [&] (auto atlasTag) -> Result<LodFont> {
        LodFont result;
        BlobInputStream stream(blob);
        co_await deserialize(stream, &result._header, tags::via<LodFontHeader_MM7>);
        co_await deserialize(stream, &result._atlas, atlasTag);
        result._pixels = stream.readAllAsBlob();
        co_await fixAndValidateFont(blob, result);
        co_return result;
    };

    Result<LodFont> result = tryDecode(tags::via<LodFontAtlas_MM7>);
    if (!result)
        if (Result<LodFont> mmx = tryDecode(tags::via<LodFontAtlas_MMX>))
            return mmx;
    return result; // If both layouts failed then report the error from the first one, it's the more likely one.
}
