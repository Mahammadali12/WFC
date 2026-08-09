#ifndef TILES_H
#define TILES_H

#include <stdbool.h>
#include <stdint.h>

#define TILE_COUNT 12

typedef enum {
    EDGE_EMPTY = 0,
    EDGE_ROAD = 1
} EdgeType;

typedef enum {
    DIR_TOP = 0,
    DIR_BOTTOM = 1,
    DIR_LEFT = 2,
    DIR_RIGHT = 3
} Direction;

typedef enum {
    TILE_HR = 0,
    TILE_VR = 1,
    TILE_UP_L = 2,
    TILE_UP_R = 3,
    TILE_LOW_L = 4,
    TILE_LOW_R = 5,
    TILE_EMPTY = 6,
    TILE_CROSS = 7,
    TILE_T_UP = 8,
    TILE_T_DOWN = 9,
    TILE_T_LEFT = 10,
    TILE_T_RIGHT = 11
} TileType;

typedef struct {
    EdgeType top;
    EdgeType bottom;
    EdgeType left;
    EdgeType right;
} TileEdges;

extern const TileEdges TILE_EDGES[TILE_COUNT];
extern const char *TILE_NAMES[TILE_COUNT];
extern const int TILE_WEIGHTS[TILE_COUNT];

EdgeType get_edge(TileType tile, Direction direction);
Direction get_opposite_direction(Direction direction);
bool can_be_adjacent(TileType tile1, TileType tile2, Direction dir);
int tile_popcount(uint32_t mask);
TileType pick_weighted_tile(uint32_t possible_tiles);

#endif // TILES_H