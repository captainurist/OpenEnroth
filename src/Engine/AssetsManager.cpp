#include "Engine/AssetsManager.h"

#include <memory>
#include <string>

#include "Engine.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Resources/LodTextureCache.h"
#include "Engine/Resources/LodSpriteCache.h"

#include "GUI/GUIFont.h"

#include "Library/Logger/Logger.h"
#include "Resources/ResourceManager.h"

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
    for (auto img : images) {
        img.second->releaseRenderId();
    }
    for (auto bit : bitmaps) {
        bit.second->releaseRenderId();
    }
    for (auto spr : sprites) {
        spr.second->releaseRenderId();
    }

    ReloadFonts();
}

bool AssetsManager::releaseImage(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        return false;
    }

    i->second->releaseRenderId();
    images.erase(filename);
    return true;
}

GraphicsImage *AssetsManager::getIcon(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        RgbaImage image = engine->resources()->icon(name);
        if (!image)
            return nullptr;
        GraphicsImage *result = GraphicsImage::Create(name, std::move(image));
        images[filename] = result;
        return result;
    }

    return i->second;
}

GraphicsImage *AssetsManager::getImage_Buff(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = images.find(filename);
    if (i == images.end()) {
        auto image = GraphicsImage::Create(std::make_unique<Buff_LOD_Loader>(pIcons_LOD, filename));
        images[filename] = image;
        return image;
    }

    return i->second;
}

GraphicsImage *AssetsManager::getBitmap(std::string_view name, bool generated) {
    std::string filename = ascii::toLower(name);

    auto i = bitmaps.find(filename);
    if (i == bitmaps.end()) {
        RgbaImage image;
        if (generated) {
            image = engine->resources()->generated(name);
        } else {
            image = engine->resources()->bitmap(name);
        }
        if (!image)
            return nullptr;
        GraphicsImage *result = GraphicsImage::Create(name, std::move(image));
        images[filename] = result;
        return result;
    }

    return i->second;
}

bool AssetsManager::releaseBitmap(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = bitmaps.find(filename);
    if (i == bitmaps.end()) {
        return false;
    }

    i->second->releaseRenderId();
    bitmaps.erase(filename);
    return true;
}

GraphicsImage *AssetsManager::getSprite(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = sprites.find(filename);
    if (i == sprites.end()) {
        RgbaImage image = engine->resources()->sprite(name);
        if (!image)
            return nullptr;
        GraphicsImage *result = GraphicsImage::Create(name, std::move(image));
        images[filename] = result;
        return result;
    }

    return i->second;
}

bool AssetsManager::releaseSprite(std::string_view name) {
    std::string filename = ascii::toLower(name);

    auto i = sprites.find(filename);
    if (i == sprites.end()) {
        return false;
    }

    i->second->releaseRenderId();
    sprites.erase(filename);
    return true;
}

