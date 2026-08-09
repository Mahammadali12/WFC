#include "tiles.h"

#include <stdlib.h>

// Edge table: top, bottom, left, right
// 58 tiles — full enumeration of the road/water/bank system.
// See docs/tile-taxonomy.md for the derivation.
const TileEdges TILE_EDGES[TILE_COUNT] = {
    // --- Road base (0-6) ---
    [TILE_HR]      = {EDGE_EMPTY, EDGE_EMPTY, EDGE_ROAD,  EDGE_ROAD},
    [TILE_VR]      = {EDGE_ROAD,  EDGE_ROAD,  EDGE_EMPTY, EDGE_EMPTY},
    [TILE_UP_L]    = {EDGE_EMPTY, EDGE_ROAD,  EDGE_EMPTY, EDGE_ROAD},
    [TILE_UP_R]    = {EDGE_EMPTY, EDGE_ROAD,  EDGE_ROAD,  EDGE_EMPTY},
    [TILE_LOW_L]   = {EDGE_ROAD,  EDGE_EMPTY, EDGE_EMPTY, EDGE_ROAD},
    [TILE_LOW_R]   = {EDGE_ROAD,  EDGE_EMPTY, EDGE_ROAD,  EDGE_EMPTY},
    [TILE_EMPTY]   = {EDGE_EMPTY, EDGE_EMPTY, EDGE_EMPTY, EDGE_EMPTY},

    // --- Crossings + T-junctions (7-11) ---
    [TILE_CROSS]   = {EDGE_ROAD,  EDGE_ROAD,  EDGE_ROAD,  EDGE_ROAD},
    [TILE_T_UP]    = {EDGE_ROAD,  EDGE_EMPTY, EDGE_ROAD,  EDGE_ROAD},
    [TILE_T_DOWN]  = {EDGE_EMPTY, EDGE_ROAD,  EDGE_ROAD,  EDGE_ROAD},
    [TILE_T_LEFT]  = {EDGE_ROAD,  EDGE_ROAD,  EDGE_ROAD,  EDGE_EMPTY},
    [TILE_T_RIGHT] = {EDGE_ROAD,  EDGE_ROAD,  EDGE_EMPTY, EDGE_ROAD},

    // --- Water base (12-17) ---
    [TILE_WATER_HR]      = {EDGE_EMPTY, EDGE_EMPTY, EDGE_WATER, EDGE_WATER},
    [TILE_WATER_VR]      = {EDGE_WATER, EDGE_WATER, EDGE_EMPTY, EDGE_EMPTY},
    [TILE_WATER_UP_L]    = {EDGE_EMPTY, EDGE_WATER, EDGE_EMPTY, EDGE_WATER},
    [TILE_WATER_UP_R]    = {EDGE_EMPTY, EDGE_WATER, EDGE_WATER, EDGE_EMPTY},
    [TILE_WATER_LOW_L]   = {EDGE_WATER, EDGE_EMPTY, EDGE_EMPTY, EDGE_WATER},
    [TILE_WATER_LOW_R]   = {EDGE_WATER, EDGE_EMPTY, EDGE_WATER, EDGE_EMPTY},

    // --- Road straight bank variants (18-23) ---
    [TILE_HR_BANK_TOP]    = {EDGE_BANK,  EDGE_EMPTY, EDGE_ROAD,  EDGE_ROAD},
    [TILE_HR_BANK_BOTTOM] = {EDGE_EMPTY, EDGE_BANK,  EDGE_ROAD,  EDGE_ROAD},
    [TILE_HR_BANK_BOTH]   = {EDGE_BANK,  EDGE_BANK,  EDGE_ROAD,  EDGE_ROAD},
    [TILE_VR_BANK_LEFT]   = {EDGE_ROAD,  EDGE_ROAD,  EDGE_BANK,  EDGE_EMPTY},
    [TILE_VR_BANK_RIGHT]  = {EDGE_ROAD,  EDGE_ROAD,  EDGE_EMPTY, EDGE_BANK},
    [TILE_VR_BANK_BOTH]   = {EDGE_ROAD,  EDGE_ROAD,  EDGE_BANK,  EDGE_BANK},

    // --- Water straight bank variants (24-29) ---
    [TILE_WATER_HR_BANK_TOP]    = {EDGE_BANK,  EDGE_EMPTY, EDGE_WATER, EDGE_WATER},
    [TILE_WATER_HR_BANK_BOTTOM] = {EDGE_EMPTY, EDGE_BANK,  EDGE_WATER, EDGE_WATER},
    [TILE_WATER_HR_BANK_BOTH]   = {EDGE_BANK,  EDGE_BANK,  EDGE_WATER, EDGE_WATER},
    [TILE_WATER_VR_BANK_LEFT]   = {EDGE_WATER, EDGE_WATER, EDGE_BANK,  EDGE_EMPTY},
    [TILE_WATER_VR_BANK_RIGHT]  = {EDGE_WATER, EDGE_WATER, EDGE_EMPTY, EDGE_BANK},
    [TILE_WATER_VR_BANK_BOTH]   = {EDGE_WATER, EDGE_WATER, EDGE_BANK,  EDGE_BANK},

    // --- Road corner bank variants (30-41) ---
    [TILE_UP_L_BANK_TOP]      = {EDGE_BANK,  EDGE_ROAD,  EDGE_EMPTY, EDGE_ROAD},
    [TILE_UP_L_BANK_LEFT]     = {EDGE_EMPTY, EDGE_ROAD,  EDGE_BANK,  EDGE_ROAD},
    [TILE_UP_L_BANK_BOTH]     = {EDGE_BANK,  EDGE_ROAD,  EDGE_BANK,  EDGE_ROAD},
    [TILE_UP_R_BANK_TOP]      = {EDGE_BANK,  EDGE_ROAD,  EDGE_ROAD,  EDGE_EMPTY},
    [TILE_UP_R_BANK_RIGHT]    = {EDGE_EMPTY, EDGE_ROAD,  EDGE_ROAD,  EDGE_BANK},
    [TILE_UP_R_BANK_BOTH]     = {EDGE_BANK,  EDGE_ROAD,  EDGE_ROAD,  EDGE_BANK},
    [TILE_LOW_L_BANK_BOTTOM]  = {EDGE_ROAD,  EDGE_BANK,  EDGE_EMPTY, EDGE_ROAD},
    [TILE_LOW_L_BANK_LEFT]    = {EDGE_ROAD,  EDGE_EMPTY, EDGE_BANK,  EDGE_ROAD},
    [TILE_LOW_L_BANK_BOTH]    = {EDGE_ROAD,  EDGE_BANK,  EDGE_BANK,  EDGE_ROAD},
    [TILE_LOW_R_BANK_BOTTOM]  = {EDGE_ROAD,  EDGE_BANK,  EDGE_ROAD,  EDGE_EMPTY},
    [TILE_LOW_R_BANK_RIGHT]   = {EDGE_ROAD,  EDGE_EMPTY, EDGE_ROAD,  EDGE_BANK},
    [TILE_LOW_R_BANK_BOTH]    = {EDGE_ROAD,  EDGE_BANK,  EDGE_ROAD,  EDGE_BANK},

    // --- Water corner bank variants (42-53) ---
    [TILE_WATER_UP_L_BANK_TOP]      = {EDGE_BANK,  EDGE_WATER, EDGE_EMPTY, EDGE_WATER},
    [TILE_WATER_UP_L_BANK_LEFT]     = {EDGE_EMPTY, EDGE_WATER, EDGE_BANK,  EDGE_WATER},
    [TILE_WATER_UP_L_BANK_BOTH]     = {EDGE_BANK,  EDGE_WATER, EDGE_BANK,  EDGE_WATER},
    [TILE_WATER_UP_R_BANK_TOP]      = {EDGE_BANK,  EDGE_WATER, EDGE_WATER, EDGE_EMPTY},
    [TILE_WATER_UP_R_BANK_RIGHT]    = {EDGE_EMPTY, EDGE_WATER, EDGE_WATER, EDGE_BANK},
    [TILE_WATER_UP_R_BANK_BOTH]     = {EDGE_BANK,  EDGE_WATER, EDGE_WATER, EDGE_BANK},
    [TILE_WATER_LOW_L_BANK_BOTTOM]  = {EDGE_WATER, EDGE_BANK,  EDGE_EMPTY, EDGE_WATER},
    [TILE_WATER_LOW_L_BANK_LEFT]    = {EDGE_WATER, EDGE_EMPTY, EDGE_BANK,  EDGE_WATER},
    [TILE_WATER_LOW_L_BANK_BOTH]    = {EDGE_WATER, EDGE_BANK,  EDGE_BANK,  EDGE_WATER},
    [TILE_WATER_LOW_R_BANK_BOTTOM]  = {EDGE_WATER, EDGE_BANK,  EDGE_WATER, EDGE_EMPTY},
    [TILE_WATER_LOW_R_BANK_RIGHT]   = {EDGE_WATER, EDGE_EMPTY, EDGE_WATER, EDGE_BANK},
    [TILE_WATER_LOW_R_BANK_BOTH]    = {EDGE_WATER, EDGE_BANK,  EDGE_WATER, EDGE_BANK},

    // --- Grass bank tiles (54-57) ---
    [TILE_GRASS_BANK_TOP]    = {EDGE_BANK,  EDGE_EMPTY, EDGE_EMPTY, EDGE_EMPTY},
    [TILE_GRASS_BANK_BOTTOM] = {EDGE_EMPTY, EDGE_BANK,  EDGE_EMPTY, EDGE_EMPTY},
    [TILE_GRASS_BANK_LEFT]   = {EDGE_EMPTY, EDGE_EMPTY, EDGE_BANK,  EDGE_EMPTY},
    [TILE_GRASS_BANK_RIGHT]  = {EDGE_EMPTY, EDGE_EMPTY, EDGE_EMPTY, EDGE_BANK},
};

const char *TILE_NAMES[TILE_COUNT] = {
    // --- Road base ---
    [TILE_HR]      = "Horizontal",
    [TILE_VR]      = "Vertical",
    [TILE_UP_L]    = "Upper-Left",
    [TILE_UP_R]    = "Upper-Right",
    [TILE_LOW_L]   = "Lower-Left",
    [TILE_LOW_R]   = "Lower-Right",
    [TILE_EMPTY]   = "Empty",
    // --- Crossings + T-junctions ---
    [TILE_CROSS]   = "Crossing",
    [TILE_T_UP]    = "T-Junction Up",
    [TILE_T_DOWN]  = "T-Junction Down",
    [TILE_T_LEFT]  = "T-Junction Left",
    [TILE_T_RIGHT] = "T-Junction Right",
    // --- Water base ---
    [TILE_WATER_HR]      = "Water Horizontal",
    [TILE_WATER_VR]      = "Water Vertical",
    [TILE_WATER_UP_L]    = "Water Upper-Left",
    [TILE_WATER_UP_R]    = "Water Upper-Right",
    [TILE_WATER_LOW_L]   = "Water Lower-Left",
    [TILE_WATER_LOW_R]   = "Water Lower-Right",
    // --- Road straight bank variants ---
    [TILE_HR_BANK_TOP]    = "HR bank top",
    [TILE_HR_BANK_BOTTOM] = "HR bank bottom",
    [TILE_HR_BANK_BOTH]   = "HR causeway",
    [TILE_VR_BANK_LEFT]   = "VR bank left",
    [TILE_VR_BANK_RIGHT]  = "VR bank right",
    [TILE_VR_BANK_BOTH]   = "VR causeway",
    // --- Water straight bank variants ---
    [TILE_WATER_HR_BANK_TOP]    = "Water HR bank top",
    [TILE_WATER_HR_BANK_BOTTOM] = "Water HR bank bottom",
    [TILE_WATER_HR_BANK_BOTH]   = "Water HR bank both",
    [TILE_WATER_VR_BANK_LEFT]   = "Water VR bank left",
    [TILE_WATER_VR_BANK_RIGHT]  = "Water VR bank right",
    [TILE_WATER_VR_BANK_BOTH]   = "Water VR bank both",
    // --- Road corner bank variants ---
    [TILE_UP_L_BANK_TOP]      = "UP_L bank top",
    [TILE_UP_L_BANK_LEFT]     = "UP_L bank left",
    [TILE_UP_L_BANK_BOTH]     = "UP_L bank both",
    [TILE_UP_R_BANK_TOP]      = "UP_R bank top",
    [TILE_UP_R_BANK_RIGHT]    = "UP_R bank right",
    [TILE_UP_R_BANK_BOTH]     = "UP_R bank both",
    [TILE_LOW_L_BANK_BOTTOM]  = "LOW_L bank bottom",
    [TILE_LOW_L_BANK_LEFT]    = "LOW_L bank left",
    [TILE_LOW_L_BANK_BOTH]    = "LOW_L bank both",
    [TILE_LOW_R_BANK_BOTTOM]  = "LOW_R bank bottom",
    [TILE_LOW_R_BANK_RIGHT]   = "LOW_R bank right",
    [TILE_LOW_R_BANK_BOTH]    = "LOW_R bank both",
    // --- Water corner bank variants ---
    [TILE_WATER_UP_L_BANK_TOP]      = "Water UP_L bank top",
    [TILE_WATER_UP_L_BANK_LEFT]     = "Water UP_L bank left",
    [TILE_WATER_UP_L_BANK_BOTH]     = "Water UP_L bank both",
    [TILE_WATER_UP_R_BANK_TOP]      = "Water UP_R bank top",
    [TILE_WATER_UP_R_BANK_RIGHT]    = "Water UP_R bank right",
    [TILE_WATER_UP_R_BANK_BOTH]     = "Water UP_R bank both",
    [TILE_WATER_LOW_L_BANK_BOTTOM]  = "Water LOW_L bank bottom",
    [TILE_WATER_LOW_L_BANK_LEFT]    = "Water LOW_L bank left",
    [TILE_WATER_LOW_L_BANK_BOTH]    = "Water LOW_L bank both",
    [TILE_WATER_LOW_R_BANK_BOTTOM]  = "Water LOW_R bank bottom",
    [TILE_WATER_LOW_R_BANK_RIGHT]   = "Water LOW_R bank right",
    [TILE_WATER_LOW_R_BANK_BOTH]    = "Water LOW_R bank both",
    // --- Grass bank tiles ---
    [TILE_GRASS_BANK_TOP]    = "Grass bank top",
    [TILE_GRASS_BANK_BOTTOM] = "Grass bank bottom",
    [TILE_GRASS_BANK_LEFT]   = "Grass bank left",
    [TILE_GRASS_BANK_RIGHT]  = "Grass bank right",
};

// Relative likelihood of each tile being picked when collapsing a cell.
// Roads + empty + water are common; bank variants and corners less so.
// Within a category, we keep the same relative weights as the existing
// tile set, so existing generations look the same modulo new tiles.
const int TILE_WEIGHTS[TILE_COUNT] = {
    // Road base
    [TILE_HR]      = 10,
    [TILE_VR]      = 10,
    [TILE_UP_L]    = 3,
    [TILE_UP_R]    = 3,
    [TILE_LOW_L]   = 3,
    [TILE_LOW_R]   = 3,
    [TILE_EMPTY]   = 12,
    // Crossings + T-junctions
    [TILE_CROSS]   = 2,
    [TILE_T_UP]    = 2,
    [TILE_T_DOWN]  = 2,
    [TILE_T_LEFT]  = 2,
    [TILE_T_RIGHT] = 2,
    // Water base — water is common, mirrors road weights but slightly lower
    [TILE_WATER_HR]      = 8,
    [TILE_WATER_VR]      = 8,
    [TILE_WATER_UP_L]    = 3,
    [TILE_WATER_UP_R]    = 3,
    [TILE_WATER_LOW_L]   = 3,
    [TILE_WATER_LOW_R]   = 3,
    // Road straight bank variants — slightly rarer than plain roads
    [TILE_HR_BANK_TOP]    = 5,
    [TILE_HR_BANK_BOTTOM] = 5,
    [TILE_HR_BANK_BOTH]   = 2,
    [TILE_VR_BANK_LEFT]   = 5,
    [TILE_VR_BANK_RIGHT]  = 5,
    [TILE_VR_BANK_BOTH]   = 2,
    // Water straight bank variants
    [TILE_WATER_HR_BANK_TOP]    = 4,
    [TILE_WATER_HR_BANK_BOTTOM] = 4,
    [TILE_WATER_HR_BANK_BOTH]   = 2,
    [TILE_WATER_VR_BANK_LEFT]   = 4,
    [TILE_WATER_VR_BANK_RIGHT]  = 4,
    [TILE_WATER_VR_BANK_BOTH]   = 2,
    // Road corner bank variants
    [TILE_UP_L_BANK_TOP]      = 2,
    [TILE_UP_L_BANK_LEFT]     = 2,
    [TILE_UP_L_BANK_BOTH]     = 1,
    [TILE_UP_R_BANK_TOP]      = 2,
    [TILE_UP_R_BANK_RIGHT]    = 2,
    [TILE_UP_R_BANK_BOTH]     = 1,
    [TILE_LOW_L_BANK_BOTTOM]  = 2,
    [TILE_LOW_L_BANK_LEFT]    = 2,
    [TILE_LOW_L_BANK_BOTH]    = 1,
    [TILE_LOW_R_BANK_BOTTOM]  = 2,
    [TILE_LOW_R_BANK_RIGHT]   = 2,
    [TILE_LOW_R_BANK_BOTH]    = 1,
    // Water corner bank variants
    [TILE_WATER_UP_L_BANK_TOP]      = 2,
    [TILE_WATER_UP_L_BANK_LEFT]     = 2,
    [TILE_WATER_UP_L_BANK_BOTH]     = 1,
    [TILE_WATER_UP_R_BANK_TOP]      = 2,
    [TILE_WATER_UP_R_BANK_RIGHT]    = 2,
    [TILE_WATER_UP_R_BANK_BOTH]     = 1,
    [TILE_WATER_LOW_L_BANK_BOTTOM]  = 2,
    [TILE_WATER_LOW_L_BANK_LEFT]    = 2,
    [TILE_WATER_LOW_L_BANK_BOTH]    = 1,
    [TILE_WATER_LOW_R_BANK_BOTTOM]  = 2,
    [TILE_WATER_LOW_R_BANK_RIGHT]   = 2,
    [TILE_WATER_LOW_R_BANK_BOTH]    = 1,
    // Grass bank tiles
    [TILE_GRASS_BANK_TOP]    = 4,
    [TILE_GRASS_BANK_BOTTOM] = 4,
    [TILE_GRASS_BANK_LEFT]   = 4,
    [TILE_GRASS_BANK_RIGHT]  = 4,
};

EdgeType get_edge(TileType tile, Direction direction)
{
    switch (direction)
    {
        case DIR_TOP:    return TILE_EDGES[tile].top;
        case DIR_BOTTOM: return TILE_EDGES[tile].bottom;
        case DIR_LEFT:   return TILE_EDGES[tile].left;
        case DIR_RIGHT:  return TILE_EDGES[tile].right;
    }
    return EDGE_EMPTY; // fallback, should never hit
}

Direction get_opposite_direction(Direction direction)
{
    switch (direction)
    {
        case DIR_TOP:    return DIR_BOTTOM;
        case DIR_BOTTOM: return DIR_TOP;
        case DIR_LEFT:   return DIR_RIGHT;
        case DIR_RIGHT:  return DIR_LEFT;
    }
    return DIR_TOP; // fallback, should never hit
}

// Strict edge equality across all four edge types.
// Why equality still works with 4 types: a contact is "valid" iff the two
// cells agree on the shared edge. EMPTY-EMPTY = grass next to grass, ROAD-ROAD
// = road continues, WATER-WATER = water continues, BANK-BANK = continuous bank
// strip. There is no case where we need an asymmetric rule (e.g. BANK matching
// EMPTY) because every cell that needs to face a BANK edge is given its own
// bank-grass tile variant (see GRASS_BANK_* in the taxonomy doc).
bool can_be_adjacent(TileType tile1, TileType tile2, Direction dir)
{
    return get_edge(tile1, dir) == get_edge(tile2, get_opposite_direction(dir));
}

int tile_popcount(uint64_t mask)
{
    int count = 0;
    for (int i = 0; i < TILE_COUNT; i++)
        if (mask & (1ULL << i))
            count++;
    return count;
}

// Weighted pick among the tiles present in the mask.
TileType pick_weighted_tile(uint64_t possible_tiles)
{
    int total = 0;
    for (int t = 0; t < TILE_COUNT; t++)
        if (possible_tiles & (1ULL << t))
            total += TILE_WEIGHTS[t];

    int r = rand() % total;
    for (int t = 0; t < TILE_COUNT; t++)
    {
        if (!(possible_tiles & (1ULL << t)))
            continue;
        if (r < TILE_WEIGHTS[t])
            return (TileType)t;
        r -= TILE_WEIGHTS[t];
    }
    return TILE_EMPTY; // unreachable when total > 0
}
