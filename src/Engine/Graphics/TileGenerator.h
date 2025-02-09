#pragma once

#include "Engine/Data/"

class TileGenerator {
public:
    TileGenerator();

    int tileId(Tileset tileset, TileVariant variant) const;
};

extern TileGenerator *pTileGenerator;
