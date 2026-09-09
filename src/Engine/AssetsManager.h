#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "Library/Color/ColorTable.h"
#include "GUI/GUIFont.h"
#include "Utility/String/TransparentFunctors.h"

class GraphicsImage;

class AssetsManager {
 public:
    AssetsManager() {}

    // TODO(captainurist): this releases all gpu-side textures that are stored in this class, but doesn't release
    //                     any of the loose textures created in other places.
    void releaseAllTextures();

    std::shared_ptr<GraphicsImage> getImage_ColorKey(std::string_view name, Color colorkey = colorTable.TealMask);
    std::shared_ptr<GraphicsImage> getImage_Paletted(std::string_view name);
    std::shared_ptr<GraphicsImage> getImage_Solid(std::string_view name);
    std::shared_ptr<GraphicsImage> getImage_Alpha(std::string_view name);
    std::shared_ptr<GraphicsImage> getImage_Buff(std::string_view name);

    std::shared_ptr<GraphicsImage> getImage_PCXFromIconsLOD(std::string_view name);

    std::shared_ptr<GraphicsImage> getBitmap(std::string_view name, bool generated = false);
    std::shared_ptr<GraphicsImage> getSprite(std::string_view name);

    std::unique_ptr<GUIFont> pFontBookOnlyShadow;
    std::unique_ptr<GUIFont> pFontBookLloyds;
    std::unique_ptr<GUIFont> pFontArrus;
    std::unique_ptr<GUIFont> pFontLucida;
    std::unique_ptr<GUIFont> pFontBookTitle;
    std::unique_ptr<GUIFont> pFontBookCalendar;
    std::unique_ptr<GUIFont> pFontCreate;
    std::unique_ptr<GUIFont> pFontCChar;
    std::unique_ptr<GUIFont> pFontComic;
    std::unique_ptr<GUIFont> pFontSmallnum;

 protected:
    std::unordered_map<std::string, std::weak_ptr<GraphicsImage>, TransparentStringHash, TransparentStringEquals> _bitmaps;
    std::unordered_map<std::string, std::weak_ptr<GraphicsImage>, TransparentStringHash, TransparentStringEquals> _sprites;
    std::unordered_map<std::string, std::weak_ptr<GraphicsImage>, TransparentStringHash, TransparentStringEquals> _icons;
};

extern AssetsManager *assets;
