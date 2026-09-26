#include "PaletteManager.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "Engine/Resources/LodTextureCache.h"
#include "Engine/Engine.h"

#include "Library/Color/Color.h"
#include "Library/Color/HsvColorf.h"
#include "Library/Color/HsvProbe.h"
#include "Library/LodFormats/LodImage.h"
#include "Library/Logger/Logger.h"

#include "Utility/String/Format.h"


PaletteManager *pPaletteManager = new PaletteManager;

void PaletteManager::load(LodTextureCache *lod) {
    const char *repeatEnv = std::getenv("OE_PALETTE_REPEAT");
    int repeat = repeatEnv ? std::max(1, std::atoi(repeatEnv)) : 1;
    int loaded = 0;
    for (int pass = 0; pass < repeat; pass++) {
    g_hsvProbePass = pass;
    loaded = 0;
    _palettes.clear();
    _palettes.reserve(1000);

    // Palette #0 is grayscale.
    _palettes.emplace_back(createGrayscalePalette());

    // Load all other palettes.
    for (int paletteId = 1; paletteId <= 999; paletteId++) {
        std::string paletteName = fmt::format("pal{:03}", paletteId);

        LodImage *texture = lod->loadTexture(paletteName, false);
        if (texture) {
            _palettes.emplace_back(createLoadedPalette(texture->palette));
            loaded++;
        } else {
            _palettes.emplace_back(createGrayscalePalette());
        }
    }
    }
    if (std::getenv("OE_PALETTE_REPORT"))
        std::fprintf(stderr, "HSVPROBE palettes loaded=%d passes=%d\n", loaded, repeat);
}

std::span<Color> PaletteManager::paletteData() {
    return {_palettes[0].colors.data(), _palettes.size() * _palettes[0].colors.size()};
}

Palette PaletteManager::createGrayscalePalette() {
    Palette result;
    for (int i = 0; i < 256; i++)
        result.colors[i] = Color(i, i, i, 255);
    return result;
}

Palette PaletteManager::createLoadedPalette(const Palette &palette) {
    float xs = engine->config->graphics.Saturation.value();
    float xv = engine->config->graphics.Lightness.value();

    Palette result;
#ifdef OE_HSV_PROBE_VERIFY
    for (size_t i = 0; i < 256; i++) {
        HsvColorf hsv = palette.colors[i].toHsvColorf();
        HsvColorf adj = hsv.adjusted(0, xs, xv);
        if (__builtin_expect(!hsvProbeInRange(hsv.h, hsv.s, hsv.v, hsv.a) || !hsvProbeInRange(adj.h, adj.s, adj.v, adj.a), 0))
            hsvProbeVerify(palette.colors[i], i, xs, xv, hsv, adj);
        result.colors[i] = adj.toColor();
    }
#else
    for (size_t i = 0; i < 256; i++)
        result.colors[i] = palette.colors[i].toHsvColorf().adjusted(0, xs, xv).toColor();
#endif
    return result;
}
