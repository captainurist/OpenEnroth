#include "TileData.h"

#include "TileEnumFunctions.h"
#include "Engine/Snapshots/EntitySnapshots.h"
#include "Engine/Snapshots/EnumSnapshots.h"

#include "Library/Snapshots/CommonSnapshots.h"

#include "Utility/String/Ascii.h"

static TileVariant mapToRoadTileVariant(TileVariant_MM7 variant) {
    switch (variant) {
    case TILE_VARIANT_MM7_BASE1_NSEW:       return TILE_VARIANT_ROAD_NSEW;
    case TILE_VARIANT_MM7_BASE2_NS:         return TILE_VARIANT_ROAD_NS;
    case TILE_VARIANT_MM7_BASE3_EW:         return TILE_VARIANT_ROAD_EW;
    case TILE_VARIANT_MM7_BASE4_NE:         return TILE_VARIANT_ROAD_NE;
    case TILE_VARIANT_MM7_SPECIAL1_NW:      return TILE_VARIANT_ROAD_NW;
    case TILE_VARIANT_MM7_SPECIAL2_SE:      return TILE_VARIANT_ROAD_SE;
    case TILE_VARIANT_MM7_SPECIAL3_SW:      return TILE_VARIANT_ROAD_SW;
    case TILE_VARIANT_MM7_SPECIAL4_NS_E:    return TILE_VARIANT_ROAD_NSE;
    case TILE_VARIANT_MM7_SPECIAL5_NS_W:    return TILE_VARIANT_ROAD_NSW;
    case TILE_VARIANT_MM7_SPECIAL6_EW_N:    return TILE_VARIANT_ROAD_NEW;
    case TILE_VARIANT_MM7_SPECIAL7_EW_S:    return TILE_VARIANT_ROAD_SEW;
    case TILE_VARIANT_MM7_SPECIAL8_NCAP:    return TILE_VARIANT_ROAD_N;
    case TILE_VARIANT_MM7_NE1_SE1_ECAP:     return TILE_VARIANT_ROAD_E;
    case TILE_VARIANT_MM7_SCAP:             return TILE_VARIANT_ROAD_S;
    case TILE_VARIANT_MM7_NW1_SW1_WCAP:     return TILE_VARIANT_ROAD_W;
    case TILE_VARIANT_MM7_DN:               return TILE_VARIANT_ROAD_NEW;
    case TILE_VARIANT_MM7_E1_DS:            return TILE_VARIANT_ROAD_SEW;
    case TILE_VARIANT_MM7_W1_DW:            return TILE_VARIANT_ROAD_NSW;
    case TILE_VARIANT_MM7_N1_DE:            return TILE_VARIANT_ROAD_NSE;
    case TILE_VARIANT_MM7_S1_DSW:           return TILE_VARIANT_ROAD_DSW;
    case TILE_VARIANT_MM7_XNE1_XSE1_DNE:    return TILE_VARIANT_ROAD_DNE;
    case TILE_VARIANT_MM7_DSE:              return TILE_VARIANT_ROAD_DSE;
    case TILE_VARIANT_MM7_XNW1_XSW1_DNW:    return TILE_VARIANT_ROAD_DNW;
    default:                                return TILE_VARIANT_INVALID;
    }
}

static TileVariant mapToNormalTileVariant(TileVariant_MM7 variant, std::string_view name) {
    switch (variant) {
    case TILE_VARIANT_MM7_BASE1_NSEW:       return TILE_VARIANT_BASE1;
    case TILE_VARIANT_MM7_BASE2_NS:         return TILE_VARIANT_BASE2;
    case TILE_VARIANT_MM7_BASE3_EW:         return TILE_VARIANT_BASE3;
    case TILE_VARIANT_MM7_BASE4_NE:         return TILE_VARIANT_BASE4;
    case TILE_VARIANT_MM7_SPECIAL1_NW:      return TILE_VARIANT_SPECIAL1;
    case TILE_VARIANT_MM7_SPECIAL2_SE:      return TILE_VARIANT_SPECIAL2;
    case TILE_VARIANT_MM7_SPECIAL3_SW:      return TILE_VARIANT_SPECIAL3;
    case TILE_VARIANT_MM7_SPECIAL4_NS_E:    return TILE_VARIANT_SPECIAL4;
    case TILE_VARIANT_MM7_SPECIAL5_NS_W:    return TILE_VARIANT_SPECIAL5;
    case TILE_VARIANT_MM7_SPECIAL6_EW_N:    return TILE_VARIANT_SPECIAL6;
    case TILE_VARIANT_MM7_SPECIAL7_EW_S:    return TILE_VARIANT_SPECIAL7;
    case TILE_VARIANT_MM7_SPECIAL8_NCAP:    return TILE_VARIANT_SPECIAL8;
    case TILE_VARIANT_MM7_NE1_SE1_ECAP:     return name.ends_with("ne") ? TILE_VARIANT_TRANSITION_N_E : TILE_VARIANT_TRANSITION_S_E;
    case TILE_VARIANT_MM7_SCAP:             return TILE_VARIANT_INVALID;
    case TILE_VARIANT_MM7_NW1_SW1_WCAP:     return name.ends_with("nw") ? TILE_VARIANT_TRANSITION_N_W : TILE_VARIANT_TRANSITION_S_W;
    case TILE_VARIANT_MM7_DN:               return TILE_VARIANT_INVALID;
    case TILE_VARIANT_MM7_E1_DS:            return TILE_VARIANT_TRANSITION_E;
    case TILE_VARIANT_MM7_W1_DW:            return TILE_VARIANT_TRANSITION_W;
    case TILE_VARIANT_MM7_N1_DE:            return TILE_VARIANT_TRANSITION_N;
    case TILE_VARIANT_MM7_S1_DSW:           return TILE_VARIANT_TRANSITION_S;
    case TILE_VARIANT_MM7_XNE1_XSE1_DNE:    return name.ends_with("xne") ? TILE_VARIANT_TRANSITION_NE : TILE_VARIANT_TRANSITION_SE;
    case TILE_VARIANT_MM7_DSE:              return TILE_VARIANT_INVALID;
    case TILE_VARIANT_MM7_XNW1_XSW1_DNW:    return name.ends_with("xnw") ? TILE_VARIANT_TRANSITION_NW : TILE_VARIANT_TRANSITION_SW;
    default:                                return TILE_VARIANT_INVALID;
    }
}

void reconstruct(const TileData_MM7 &src, TileData *dst) {
    reconstruct(src.tileName, &dst->name);
    dst->name = ascii::toLower(dst->name);

    if (ascii::noCaseStartsWith(dst->name, "wtrdr"))
        dst->name.insert(0, "h"); // animated water only works with hwtrdr* tiles.

    dst->tileId = src.tileId;
    dst->tileset = static_cast<Tileset>(src.tileset);
    dst->flags = static_cast<TileFlags>(src.flags);

    TileVariant_MM7 variant = static_cast<TileVariant_MM7>(src.variant);
    if (dst->tileset == TILESET_INVALID) {
        dst->variant = TILE_VARIANT_INVALID;
    } else if (isRoad(dst->tileset)) {
        dst->variant = mapToRoadTileVariant(variant);
    } else {
        dst->variant = mapToNormalTileVariant(variant, dst->name);
    }
}
