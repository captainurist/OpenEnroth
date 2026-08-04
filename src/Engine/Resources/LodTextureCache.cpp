#include "LodTextureCache.h"

#include <utility>
#include <string>

#include "Library/LodFormats/LodFormats.h"

#include "Library/Logger/Logger.h"

#include "Utility/String/Ascii.h"
#include "Utility/MapAccess.h"

LodTextureCache *pIcons_LOD = nullptr;
LodTextureCache *pIcons_LOD_mm6 = nullptr;
LodTextureCache *pIcons_LOD_mm8 = nullptr;

LodTextureCache *pBitmaps_LOD = nullptr;
LodTextureCache *pBitmaps_LOD_mm6 = nullptr;
LodTextureCache *pBitmaps_LOD_mm8 = nullptr;

LodTextureCache::LodTextureCache() = default;
LodTextureCache::~LodTextureCache() = default;

Result<void> LodTextureCache::open(Blob blob) {
    return _reader.open(std::move(blob));
}

void LodTextureCache::reserveLoadedTextures() {
    _reservedCount = _texturesInOrder.size();
}

void LodTextureCache::releaseUnreserved() {
    while (_texturesInOrder.size() > _reservedCount) {
        const std::string &name = _texturesInOrder.back();
        _textureByName.erase(name);
        _texturesInOrder.pop_back();
    }
}

LodImage *LodTextureCache::loadTexture(std::string_view pContainer, bool useDummyOnError) {
    std::string name = ascii::toLower(pContainer);

    LodImage *result = valuePtr(_textureByName, name);
    if (result)
        return result;

    result = &_textureByName[name];
    if (LoadTextureFromLOD(result, name)) {
        _texturesInOrder.push_back(name);
        return result;
    }
    _textureByName.erase(name);

    if (useDummyOnError) {
        return loadTexture("pending", false);
    } else {
        return nullptr;
    }
}

Result<Blob> LodTextureCache::LoadCompressedTexture(std::string_view pContainer) {
    Blob data = co_await _reader.read(pContainer);
    co_return lod::decodeMaybeCompressed(data);
}

Result<Blob> LodTextureCache::read(std::string_view pContainer) {
    return _reader.read(pContainer);
}

bool LodTextureCache::LoadTextureFromLOD(LodImage *pOutTex, std::string_view pContainer) {
    if (!_reader.exists(pContainer))
        return false;

    Result<Blob> data = _reader.read(pContainer);
    if (!data) {
        logger->warning("Couldn't load texture '{}': {}", pContainer, data.error());
        return false; // Caller falls back to the dummy texture.
    }

    Result<LodImage> image = lod::decodeImage(*data);
    if (!image) {
        logger->warning("Couldn't load texture '{}': {}", pContainer, image.error());
        return false; // Caller falls back to the dummy texture.
    }

    *pOutTex = *std::move(image);
    return true;
}
