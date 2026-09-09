#include "Engine/AssetsManager.h"

#include <memory>
#include <string>

#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Resources/LodTextureCache.h"
#include "Engine/Resources/LodSpriteCache.h"

#include "GUI/GUIFont.h"

#include "Library/Logger/Logger.h"
#include "Utility/MapAccess.h"

#include "Utility/String/Ascii.h"

AssetsManager *assets = new AssetsManager();

static void ReloadFonts() {
    if (assets->pFontBookOnlyShadow)
        assets->pFontBookOnlyShadow->CreateFontTex();
    if (assets->pFontBookLloyds)
        assets->pFontBookLloyds->CreateFontTex();
    if (assets->pFontArrus)
        assets->pFontArrus->CreateFontTex();
    if (assets->pFontLucida)
        assets->pFontLucida->CreateFontTex();
    if (assets->pFontBookTitle)
        assets->pFontBookTitle->CreateFontTex();
    if (assets->pFontBookCalendar)
        assets->pFontBookCalendar->CreateFontTex();
    if (assets->pFontCreate)
        assets->pFontCreate->CreateFontTex();
    if (assets->pFontCChar)
        assets->pFontCChar->CreateFontTex();
    if (assets->pFontComic)
        assets->pFontComic->CreateFontTex();
    if (assets->pFontSmallnum)
        assets->pFontSmallnum->CreateFontTex();
}

void AssetsManager::releaseAllTextures() {
    logger->trace("Render - Releasing Textures.");

    // clears any textures from gpu
    for (auto &&map : {_icons, _bitmaps, _sprites})
        for (auto &&ref : map)
            if (auto &&texture = ref.second.lock())
                texture->releaseRenderId();

    ReloadFonts();
}

std::shared_ptr<GraphicsImage> AssetsManager::getImage_Paletted(std::string_view name) {
    std::string filename = ascii::toLower(name);

    std::shared_ptr<GraphicsImage> result = valueOr(_icons, filename).lock();
    if (result)
        return result;

    result = GraphicsImage::Create(std::make_unique<Paletted_Img_Loader>(pIcons_LOD, filename));
    _icons[filename] = result;
    return result;
}


std::shared_ptr<GraphicsImage> AssetsManager::getImage_ColorKey(std::string_view name, Color colorkey) {
    std::string filename = ascii::toLower(name);

    std::shared_ptr<GraphicsImage> result = valueOr(_icons, filename).lock();
    if (result)
        return result;

    result = GraphicsImage::Create(std::make_unique<ColorKey_LOD_Loader>(pIcons_LOD, filename, colorkey));
    _icons[filename] = result;
    return result;
}

std::shared_ptr<GraphicsImage> AssetsManager::getImage_Solid(std::string_view name) {
    std::string filename = ascii::toLower(name);

    std::shared_ptr<GraphicsImage> result = valueOr(_icons, filename).lock();
    if (result)
        return result;

    result = GraphicsImage::Create(std::make_unique<Image16bit_LOD_Loader>(pIcons_LOD, filename));
    _icons[filename] = result;
    return result;
}

std::shared_ptr<GraphicsImage> AssetsManager::getImage_Alpha(std::string_view name) {
    std::string filename = ascii::toLower(name);

    std::shared_ptr<GraphicsImage> result = valueOr(_icons, filename).lock();
    if (result)
        return result;

    result = GraphicsImage::Create(std::make_unique<Alpha_LOD_Loader>(pIcons_LOD, filename));
    _icons[filename] = result;
    return result;
}

std::shared_ptr<GraphicsImage> AssetsManager::getImage_Buff(std::string_view name) {
    std::string filename = ascii::toLower(name);

    std::shared_ptr<GraphicsImage> result = valueOr(_icons, filename).lock();
    if (result)
        return result;

    result = GraphicsImage::Create(std::make_unique<Buff_LOD_Loader>(pIcons_LOD, filename));
    _icons[filename] = result;
    return result;
}

std::shared_ptr<GraphicsImage> AssetsManager::getImage_PCXFromIconsLOD(std::string_view name) {
    std::string filename = ascii::toLower(name);

    std::shared_ptr<GraphicsImage> result = valueOr(_icons, filename).lock();
    if (result)
        return result;

    result = GraphicsImage::Create(std::make_unique<PCX_LOD_Compressed_Loader>(pIcons_LOD, filename));
    _icons[filename] = result;
    return result;
}

std::shared_ptr<GraphicsImage> AssetsManager::getBitmap(std::string_view name, bool generated) {
    std::string filename = ascii::toLower(name);

    std::shared_ptr<GraphicsImage> result = valueOr(_bitmaps, filename).lock();
    if (result)
        return result;

    if (generated) {
        result = GraphicsImage::Create(std::make_unique<Bitmaps_GEN_Loader>(filename));
    } else {
        result = GraphicsImage::Create(std::make_unique<Bitmaps_LOD_Loader>(pBitmaps_LOD, filename));
    }
    _icons[filename] = result;
    return result;
}

std::shared_ptr<GraphicsImage> AssetsManager::getSprite(std::string_view name) {
    std::string filename = ascii::toLower(name);

    std::shared_ptr<GraphicsImage> result = valueOr(_sprites, filename).lock();
    if (result)
        return result;

    result = GraphicsImage::Create(std::make_unique<Sprites_LOD_Loader>(pSprites_LOD, filename));
    _icons[filename] = result;
    return result;
}

