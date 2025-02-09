#pragma once

#include <cstdint>
#include <string>

#include "TileEnums.h"

struct TileData_MM7;

struct TileData {
    std::string name;
    uint16_t tileId = 0;
    Tileset tileset = TILESET_INVALID;
    TileVariant variant = TILE_VARIANT_BASE1;
    TileFlags flags;
};

void reconstruct(const TileData_MM7 &src, TileData *dst);
