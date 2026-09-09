#include "PaletteManager.h"

#include <algorithm>
#include <string>

#include "Engine/Resources/LodTextureCache.h"
#include "Engine/Engine.h"

#include "Library/Color/Color.h"
#include "Library/LodFormats/LodImage.h"
#include "Library/Logger/Logger.h"

#include "Utility/String/Format.h"

static Palette createGrayscalePalette() {
    Palette result;
    for (int i = 0; i < 256; i++)
        result.colors[i] = Color(i, i, i, 255);
    return result;
}

PaletteManager *pPaletteManager = new PaletteManager;

void PaletteManager::load(LodTextureCache *lod) {
    _palettes.clear();
    _palettes.reserve(1000);

    // Palette #0 is grayscale.
    _palettes.emplace_back(createGrayscalePalette());

    // Load all other palettes.
    for (int paletteId = 1; paletteId <= 999; paletteId++) {
        std::string paletteName = fmt::format("pal{:03}", paletteId);

        LodImage *texture = lod->loadTexture(paletteName, false);
        if (texture) {
            desaturate(&texture->palette);
            _palettes.emplace_back(texture->palette);
        } else {
            _palettes.emplace_back(createGrayscalePalette());
        }
    }
}

std::span<Color> PaletteManager::paletteData() {
    return {_palettes[0].colors.data(), _palettes.size() * _palettes[0].colors.size()};
}

void PaletteManager::desaturate(Palette *palette) {
    float xs = engine->config->graphics.Saturation.value();
    float xv = engine->config->graphics.Lightness.value();

    for (Color &color : palette->colors)
        color = color.toHsvColorf().adjusted(0, xs, xv).toColor();
}

void PaletteManager::desaturate(RgbaImage *image) {
    float xs = engine->config->graphics.Saturation.value();
    float xv = engine->config->graphics.Lightness.value();
    for (Color &pixel : image->pixels())
        pixel = pixel.toHsvColorf().adjusted(0, xs, xv).toColor();
}
