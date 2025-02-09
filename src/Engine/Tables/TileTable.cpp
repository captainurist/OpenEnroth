#include "TileTable.h"

#include "Engine/AssetsManager.h"
#include "Engine/Random/Random.h"
#include "Engine/Data/TileEnumFunctions.h"
#include "Engine/Snapshots/TableSerialization.h"
#include "Engine/Snapshots/EntitySnapshots.h"

#include "Library/Binary/BinaryTags.h"
#include "Library/Snapshots/SnapshotSerialization.h"

TileTable *pTileTable;

int TileTable::tileIdForTileset(Tileset terrain_type, bool nonRandom) const {
    int v5;  // edx@3
    int v6;  // edx@11

    if (nonRandom || terrain_type > TILESET_TROPICAL) {
        return tileId(terrain_type, TILE_VARIANT_BASE1);
    }
    v5 = vrng->random(50);
    if (v5 < 20) {
        return tileId(terrain_type, TILE_VARIANT_BASE1);
    } else if (v5 < 30) {
        return tileId(terrain_type, TILE_VARIANT_BASE2);
    } else if (v5 < 40) {
        return tileId(terrain_type, TILE_VARIANT_BASE3);
    } else if (v5 < 48) {
        return tileId(terrain_type, TILE_VARIANT_BASE4);
    }
    return tileId(terrain_type, vrng->randomSample(allSpecialTileVariants()));
}

int TileTable::tileId(Tileset tileset, TileVariant section) const {
    for (size_t i = 0; i < _tiles.size(); ++i) {
        if ((_tiles[i].tileset == tileset) &&
            (_tiles[i].variant == section))
            return i;
    }
    return 0;
}

const TileData &TileTable::tile(int id) const {
    return _tiles[id];
}
