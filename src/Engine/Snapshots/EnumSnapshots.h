#pragma once

#include <cstdint>

enum class TileVariant_MM7 : uint16_t {
    TILE_VARIANT_MM7_INVALID = 255, // Tile with id 0 has variant = 255.
    TILE_VARIANT_MM7_BASE1_NSEW = 0, // Base tile. For roads - crossing.
    TILE_VARIANT_MM7_BASE2_NS = 1, // Base variation (swamp has one). For roads - NS.
    TILE_VARIANT_MM7_BASE3_EW = 2, // Base variation (swamp has one). For roads - EW.
    TILE_VARIANT_MM7_BASE4_NE = 3, // Base variation (not sure if this is used). For roads - NE turn.
    TILE_VARIANT_MM7_SPECIAL1_NW = 4, // For roads - NW turn.
    TILE_VARIANT_MM7_SPECIAL2_SE = 5, // For roads - SE turn.
    TILE_VARIANT_MM7_SPECIAL3_SW = 6, // For roads - SW turn.
    TILE_VARIANT_MM7_SPECIAL4_NS_E = 7, // For roads - T intersection, NS/E.
    TILE_VARIANT_MM7_SPECIAL5_NS_W = 8, // For roads - T intersection, NS/W.
    TILE_VARIANT_MM7_SPECIAL6_EW_N = 9, // For roads - T intersection, EW/N.
    TILE_VARIANT_MM7_SPECIAL7_EW_S = 0xA, // For roads - T intersection, EW/S.
    TILE_VARIANT_MM7_SPECIAL8_NCAP = 0xB, // For roads - N road end.
    TILE_VARIANT_MM7_NE1_SE1_ECAP = 0xC, // NE/SE transition to 3 x dirt. For roads - E road end.
    TILE_VARIANT_MM7_SCAP = 0xD, // For roads - S road end.
    TILE_VARIANT_MM7_NW1_SW1_WCAP = 0xE, // NW/SW transition to 3 x dirt. For roads - W road end.
    TILE_VARIANT_MM7_DN = 0xF, // For roads - Y intersection, N.
    TILE_VARIANT_MM7_E1_DS = 0x10, // E transition to dirt. For roads - Y intersection, S.
    TILE_VARIANT_MM7_W1_DW = 0x11, // W transition to dirt. For roads - Y intersection, W.
    TILE_VARIANT_MM7_N1_DE = 0x12, // N transition to dirt. For roads - Y intersection, E.
    TILE_VARIANT_MM7_S1_DSW = 0x13, // S transition to dirt. For roads - diagonal road, SW.
    TILE_VARIANT_MM7_XNE1_XSE1_DNE = 0x14, // NE/SE transition to 1 x dirt. For roads - diagonal road, NE.
    TILE_VARIANT_MM7_DSE = 0x15, // For roads - diagonal road, SE.
    TILE_VARIANT_MM7_XNW1_XSW1_DNW = 0x16, // NW/SW transition to 1 x dirt. For roads - diagonal road, NW.
};
using enum TileVariant_MM7;
static_assert(sizeof(TileVariant_MM7) == 2);
