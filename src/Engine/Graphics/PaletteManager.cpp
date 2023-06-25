#include "PaletteManager.h"

#include <algorithm>
#include <string>

#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/OurMath.h"

#include "Library/Color/Color.h"
#include "Utility/Format.h"

PaletteManager *pPaletteManager = new PaletteManager;

void PaletteManager::load(LODFile_IconsBitmaps *lod) {
    // Palette #0 is grayscale.
    _paletteIds.push_back(0);
    _palettes.push_back(createGrayscalePalette());

    // Load all other palettes.
    for (int paletteId = 1; paletteId <= 999; paletteId++) {
        std::string paletteName = fmt::format("pal{:03}", paletteId);

        Texture_MM7 texture;
        if (lod->LoadTextureFromLOD(&texture, paletteName, TEXTURE_24BIT_PALETTE) != 1)
            continue;

        _paletteIds.push_back(paletteId);
        _palettes.push_back(createLoadedPalette(texture.pPalette24));

        texture.Release();
    }
}

int PaletteManager::paletteIndex(int paletteId) {
    auto pos = std::lower_bound(_paletteIds.begin(), _paletteIds.end(), paletteId);

    if (pos == _paletteIds.end() || *pos != paletteId) {
        logger->warning("Palette {} doesn't exist. Returning index to grayscale!", paletteId);
        return 0;
    }

    return pos - _paletteIds.begin();
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

Palette PaletteManager::createLoadedPalette(uint8_t *data) {
    Palette result;

    for (int index = 0; index < 768; index += 3) {
        HsvColorf hsv = Color(data[index], data[index + 1], data[index + 2]).toColorf().toHsv();

        hsv.v = std::clamp(hsv.v * 1.1f, 0.0f, 1.0f);
        hsv.s = std::clamp(hsv.s * 0.64999998f, 0.0f, 1.0f);

        result.colors[index / 3] = hsv.toRgb().toColor();
    }

    return result;
}