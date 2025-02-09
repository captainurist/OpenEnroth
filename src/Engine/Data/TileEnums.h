#pragma once

#include "Library/Serialization/SerializationFwd.h"

#include "Utility/Flags.h"
#include "Utility/Preprocessor.h"

enum class TileFlag {
    TILE_BURN = 0x1,
    TILE_WATER = 0x2, // Literally water.
    TILE_BLOCK = 0x4,
    TILE_REPULSE = 0x8,
    TILE_FLAT = 0x10,
    TILE_WAVY = 0x20,
    TILE_DONT_DRAW = 0x40,
    TILE_SHORE = 0x100, // Shore tile that's drawn on top of the water.
    TILE_TRANSITION = 0x200, // Transition tile, e.g. dirt-sand, dirt-water, grass-dirt, etc. All road tiles have this set.
    TILE_SCROLL_DOWN = 0x400,
    TILE_SCROLL_UP = 0x800,
    TILE_SCROLL_LEFT = 0x1000,
    TILE_SCROLL_RIGHT = 0x2000,
    TILE_GENERATED_TRANSITION = 0x4000, // OE-specific, transition tile that was auto-generated, tile id might not be 
                                        // stable between runs.
};
using enum TileFlag;
MM_DECLARE_FLAGS(TileFlags, TileFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(TileFlags)

/**
 * Tile variants inside a single tile set.
 */
enum class TileVariant {
    TILE_VARIANT_INVALID = -1,

    // Base tile variants. Only swamp has variants in MM7.
    TILE_VARIANT_BASE1 = 0,
    TILE_VARIANT_BASE2,
    TILE_VARIANT_BASE3,
    TILE_VARIANT_BASE4,

    // Special tile variants. Don't exist in MM7.
    TILE_VARIANT_SPECIAL1,
    TILE_VARIANT_SPECIAL2,
    TILE_VARIANT_SPECIAL3,
    TILE_VARIANT_SPECIAL4,
    TILE_VARIANT_SPECIAL5,
    TILE_VARIANT_SPECIAL6,
    TILE_VARIANT_SPECIAL7,
    TILE_VARIANT_SPECIAL8,

    // Road tile variants.
    TILE_VARIANT_ROAD_NSEW, // Intersection.
    TILE_VARIANT_ROAD_NS, // Straight road.
    TILE_VARIANT_ROAD_EW,
    TILE_VARIANT_ROAD_NE, // 90 deg turns.
    TILE_VARIANT_ROAD_NW,
    TILE_VARIANT_ROAD_SE,
    TILE_VARIANT_ROAD_SW,
    TILE_VARIANT_ROAD_NSE, // T intersections.
    TILE_VARIANT_ROAD_NSW,
    TILE_VARIANT_ROAD_NEW,
    TILE_VARIANT_ROAD_SEW,
    TILE_VARIANT_ROAD_N, // Road ends.
    TILE_VARIANT_ROAD_S,
    TILE_VARIANT_ROAD_E,
    TILE_VARIANT_ROAD_W,
    TILE_VARIANT_ROAD_YNSE, // Y intersections with 135 deg turns.
    TILE_VARIANT_ROAD_YNSW,
    TILE_VARIANT_ROAD_YNEW,
    TILE_VARIANT_ROAD_YSEW,
    TILE_VARIANT_ROAD_DNE, // Diagonal roads.
    TILE_VARIANT_ROAD_DNW,
    TILE_VARIANT_ROAD_DSE,
    TILE_VARIANT_ROAD_DSW,

    // Transition tile variants follow, 46 total.

    // ?X?
    // ...
    // ...
    TILE_VARIANT_TRANSITION_N,
    TILE_VARIANT_TRANSITION_S,
    TILE_VARIANT_TRANSITION_E,
    TILE_VARIANT_TRANSITION_W,

    // ..X
    // ...
    // ...
    TILE_VARIANT_TRANSITION_NE,
    TILE_VARIANT_TRANSITION_NW,
    TILE_VARIANT_TRANSITION_SE,
    TILE_VARIANT_TRANSITION_SW,

    // ?X?
    // ..X
    // ..?
    TILE_VARIANT_TRANSITION_N_E,
    TILE_VARIANT_TRANSITION_N_W,
    TILE_VARIANT_TRANSITION_S_E,
    TILE_VARIANT_TRANSITION_S_W,

    // Doesn't exist in MM7 data:
    // ?X?
    // ...
    // ?X?
    TILE_VARIANT_TRANSITION_N_S,
    TILE_VARIANT_TRANSITION_E_W,

    // Doesn't exist in MM7 data:
    // ?X?
    // ..X
    // ?X?
    TILE_VARIANT_TRANSITION_N_S_E,
    TILE_VARIANT_TRANSITION_N_S_W,
    TILE_VARIANT_TRANSITION_N_E_W,
    TILE_VARIANT_TRANSITION_S_E_W,

    // Doesn't exist in MM7 data:
    // ?X?
    // X.X
    // ?X?
    TILE_VARIANT_TRANSITION_N_S_E_W,

    // Doesn't exist in MM7 data:
    // X.X
    // ...
    // ...
    TILE_VARIANT_TRANSITION_NE_NW,
    TILE_VARIANT_TRANSITION_NE_SE,
    TILE_VARIANT_TRANSITION_NW_SW,
    TILE_VARIANT_TRANSITION_SE_SW,

    // Doesn't exist in MM7 data:
    // ..X
    // ...
    // X..
    TILE_VARIANT_TRANSITION_NE_SW,
    TILE_VARIANT_TRANSITION_NW_SE,

    // Doesn't exist in MM7 data:
    // X.X
    // ...
    // ..X
    TILE_VARIANT_TRANSITION_NE_NW_SE,
    TILE_VARIANT_TRANSITION_NE_NW_SW,
    TILE_VARIANT_TRANSITION_NE_SE_SW,
    TILE_VARIANT_TRANSITION_NW_SE_SW,

    // Doesn't exist in MM7 data:
    // X.X
    // ...
    // X.X
    TILE_VARIANT_TRANSITION_NE_NW_SE_SW,

    // Doesn't exist in MM7 data:
    // ?X?
    // ...
    // X.X
    TILE_VARIANT_TRANSITION_N_SE_SW,
    TILE_VARIANT_TRANSITION_S_NE_NW,
    TILE_VARIANT_TRANSITION_E_NW_SW,
    TILE_VARIANT_TRANSITION_W_NE_SE,

    // Doesn't exist in MM7 data:
    // ?X?
    // ...
    // ..X
    TILE_VARIANT_TRANSITION_N_SE,
    TILE_VARIANT_TRANSITION_N_SW,
    TILE_VARIANT_TRANSITION_S_NE,
    TILE_VARIANT_TRANSITION_S_NW,
    TILE_VARIANT_TRANSITION_E_NW,
    TILE_VARIANT_TRANSITION_E_SW,
    TILE_VARIANT_TRANSITION_W_NE,
    TILE_VARIANT_TRANSITION_W_SE,

    // Doesn't exist in MM7 data:
    // ?X?
    // ..X
    // X.?
    TILE_VARIANT_TRANSITION_N_E_SW,
    TILE_VARIANT_TRANSITION_N_W_SE,
    TILE_VARIANT_TRANSITION_S_E_NW,
    TILE_VARIANT_TRANSITION_S_W_NE,

    TILE_VARIANT_FIRST_SPECIAL = TILE_VARIANT_SPECIAL1,
    TILE_VARIANT_LAST_SPECIAL = TILE_VARIANT_SPECIAL8,
};
using enum TileVariant;

/**
 * Tile set id.
 *
 * Most of these tile sets don't exist in mm7 data, see comments.
 */
enum class Tileset {
    TILESET_INVALID = 255, // Tile with id 0 has tile set = 255.
    TILESET_GRASS = 0,
    TILESET_SNOW = 1,
    TILESET_DESERT = 2, // Sand.
    TILESET_COOLED_LAVA = 3, // Somehow this tileset is all dirt in the data files.
    TILESET_DIRT = 4, // This one has only 3 tiles.
    TILESET_WATER = 5, // Water tile & shoreline tiles.
    TILESET_BADLANDS = 6, // Looks like Deyja.
    TILESET_SWAMP = 7,
    TILESET_TROPICAL = 8, // This is all dirt.
    TILESET_CITY = 9, // This is sand too, lol.
    TILESET_ROAD_GRASS_COBBLE = 10, // Cobble road on dirt, actually.
    TILESET_ROAD_GRASS_DIRT = 11, // This is all dirt.
    TILESET_ROAD_SNOW_COBBLE = 12, // This is all dirt.
    TILESET_ROAD_SNOW_DIRT = 13, // This is all dirt.
    TILESET_ROAD_SAND_COBBLE = 14, // This doesn't exist in mm7 tiles at all.
    TILESET_ROAD_SAND_DIRT = 15, // This doesn't exist in mm7 tiles at all.
    TILESET_ROAD_VOLCANO_COBBLE = 16, // This is all dirt.
    TILESET_ROAD_VOLCANO_DIRT = 17, // This is all dirt.
    TILESET_ROAD_CRACKED_COBBLE = 22, // This is all dirt.
    TILESET_ROAD_CRACKED_DIRT = 23, // This is all dirt.
    TILESET_ROAD_SWAMP_COBBLE = 24, // This is all dirt.
    TILESET_ROAD_SWAMP_DIRT = 25, // This is all dirt.
    TILESET_ROAD_TROPICAL_COBBLE = 26, // This is all dirt.
    TILESET_ROAD_TROPICAL_DIRT = 27, // This is all dirt.
    TILESET_ROAD_CITY_STONE = 28, // This is all dirt.

    TILESET_FIRST_ROAD = TILESET_ROAD_GRASS_COBBLE,
    TILESET_LAST_ROAD = TILESET_ROAD_CITY_STONE,
};
using enum Tileset;
