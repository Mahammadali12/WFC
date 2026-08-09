#include "tiles.h"

#include <stdlib.h>

const TileEdges TILE_EDGES[TILE_COUNT] = {
    [TILE_HR]      = {EDGE_EMPTY, EDGE_EMPTY, EDGE_ROAD, EDGE_ROAD},
    [TILE_VR]      = {EDGE_ROAD, EDGE_ROAD, EDGE_EMPTY, EDGE_EMPTY},
    [TILE_UP_L]    = {EDGE_EMPTY, EDGE_ROAD, EDGE_EMPTY, EDGE_ROAD},
    [TILE_UP_R]    = {EDGE_EMPTY, EDGE_ROAD, EDGE_ROAD, EDGE_EMPTY},
    [TILE_LOW_L]   = {EDGE_ROAD, EDGE_EMPTY, EDGE_EMPTY, EDGE_ROAD},
    [TILE_LOW_R]   = {EDGE_ROAD, EDGE_EMPTY, EDGE_ROAD, EDGE_EMPTY},
    [TILE_EMPTY]   = {EDGE_EMPTY, EDGE_EMPTY, EDGE_EMPTY, EDGE_EMPTY},
    [TILE_CROSS]   = {EDGE_ROAD, EDGE_ROAD, EDGE_ROAD, EDGE_ROAD},
    [TILE_T_UP]    = {EDGE_ROAD, EDGE_EMPTY, EDGE_ROAD, EDGE_ROAD},
    [TILE_T_DOWN]  = {EDGE_EMPTY, EDGE_ROAD, EDGE_ROAD, EDGE_ROAD},
    [TILE_T_LEFT]  = {EDGE_ROAD, EDGE_ROAD, EDGE_ROAD, EDGE_EMPTY},
    [TILE_T_RIGHT] = {EDGE_ROAD, EDGE_ROAD, EDGE_EMPTY, EDGE_ROAD},
};

const char *TILE_NAMES[TILE_COUNT] = {
    [TILE_HR]      = "Horizontal",
    [TILE_VR]      = "Vertical",
    [TILE_UP_L]    = "Upper-Left",
    [TILE_UP_R]    = "Upper-Right",
    [TILE_LOW_L]   = "Lower-Left",
    [TILE_LOW_R]   = "Lower-Right",
    [TILE_EMPTY]   = "Empty",
    [TILE_CROSS]   = "Crossing",
    [TILE_T_UP]    = "T-Junction Up",
    [TILE_T_DOWN]  = "T-Junction Down",
    [TILE_T_LEFT]  = "T-Junction Left",
    [TILE_T_RIGHT] = "T-Junction Right",
};

// Relative likelihood of each tile being picked when collapsing a cell.
// Straights and empty dominate so maps read as road networks, not noise.
const int TILE_WEIGHTS[TILE_COUNT] = {
    [TILE_HR]      = 10,
    [TILE_VR]      = 10,
    [TILE_UP_L]    = 3,
    [TILE_UP_R]    = 3,
    [TILE_LOW_L]   = 3,
    [TILE_LOW_R]   = 3,
    [TILE_EMPTY]   = 12,
    [TILE_CROSS]   = 2,
    [TILE_T_UP]    = 2,
    [TILE_T_DOWN]  = 2,
    [TILE_T_LEFT]  = 2,
    [TILE_T_RIGHT] = 2,
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

// Strict edge equality: the edge of tile1 facing 'dir' must equal the edge of
// tile2 facing the opposite direction.
bool can_be_adjacent(TileType tile1, TileType tile2, Direction dir)
{
    return get_edge(tile1, dir) == get_edge(tile2, get_opposite_direction(dir));
}

int tile_popcount(uint32_t mask)
{
    int count = 0;
    for (int i = 0; i < TILE_COUNT; i++)
        if (mask & (1u << i))
            count++;
    return count;
}

// Weighted pick among the tiles present in the mask.
TileType pick_weighted_tile(uint32_t possible_tiles)
{
    int total = 0;
    for (int t = 0; t < TILE_COUNT; t++)
        if (possible_tiles & (1u << t))
            total += TILE_WEIGHTS[t];

    int r = rand() % total;
    for (int t = 0; t < TILE_COUNT; t++)
    {
        if (!(possible_tiles & (1u << t)))
            continue;
        if (r < TILE_WEIGHTS[t])
            return (TileType)t;
        r -= TILE_WEIGHTS[t];
    }
    return TILE_EMPTY; // unreachable when total > 0
}