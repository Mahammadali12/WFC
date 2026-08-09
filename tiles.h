#ifndef TILES_H
#define TILES_H

#include <stdbool.h>
#include <stdint.h>

#define TILE_COUNT 58

typedef enum {
    EDGE_EMPTY = 0,
    EDGE_ROAD  = 1,
    EDGE_WATER = 2,
    EDGE_BANK  = 3
} EdgeType;

typedef enum {
    DIR_TOP = 0,
    DIR_BOTTOM = 1,
    DIR_LEFT = 2,
    DIR_RIGHT = 3
} Direction;

typedef enum {
    // --- Road base (7) ---
    TILE_HR = 0,
    TILE_VR = 1,
    TILE_UP_L = 2,
    TILE_UP_R = 3,
    TILE_LOW_L = 4,
    TILE_LOW_R = 5,
    TILE_EMPTY = 6,

    // --- Crossings + T-junctions (5) ---
    TILE_CROSS = 7,
    TILE_T_UP = 8,
    TILE_T_DOWN = 9,
    TILE_T_LEFT = 10,
    TILE_T_RIGHT = 11,

    // --- Water base (6) ---
    TILE_WATER_HR = 12,
    TILE_WATER_VR = 13,
    TILE_WATER_UP_L = 14,
    TILE_WATER_UP_R = 15,
    TILE_WATER_LOW_L = 16,
    TILE_WATER_LOW_R = 17,

    // --- Road straight bank variants (3) ---
    TILE_HR_BANK_TOP = 18,
    TILE_HR_BANK_BOTTOM = 19,
    TILE_HR_BANK_BOTH = 20,

    // --- VR straight bank variants (3) ---
    TILE_VR_BANK_LEFT = 21,
    TILE_VR_BANK_RIGHT = 22,
    TILE_VR_BANK_BOTH = 23,

    // --- Water straight bank variants (3) ---
    TILE_WATER_HR_BANK_TOP = 24,
    TILE_WATER_HR_BANK_BOTTOM = 25,
    TILE_WATER_HR_BANK_BOTH = 26,

    // --- Water VR straight bank variants (3) ---
    TILE_WATER_VR_BANK_LEFT = 27,
    TILE_WATER_VR_BANK_RIGHT = 28,
    TILE_WATER_VR_BANK_BOTH = 29,

    // --- Road corner bank variants: 4 corners x 3 each = 12 ---
    TILE_UP_L_BANK_TOP = 30,
    TILE_UP_L_BANK_LEFT = 31,
    TILE_UP_L_BANK_BOTH = 32,
    TILE_UP_R_BANK_TOP = 33,
    TILE_UP_R_BANK_RIGHT = 34,
    TILE_UP_R_BANK_BOTH = 35,
    TILE_LOW_L_BANK_BOTTOM = 36,
    TILE_LOW_L_BANK_LEFT = 37,
    TILE_LOW_L_BANK_BOTH = 38,
    TILE_LOW_R_BANK_BOTTOM = 39,
    TILE_LOW_R_BANK_RIGHT = 40,
    TILE_LOW_R_BANK_BOTH = 41,

    // --- Water corner bank variants: 4 corners x 3 each = 12 ---
    TILE_WATER_UP_L_BANK_TOP = 42,
    TILE_WATER_UP_L_BANK_LEFT = 43,
    TILE_WATER_UP_L_BANK_BOTH = 44,
    TILE_WATER_UP_R_BANK_TOP = 45,
    TILE_WATER_UP_R_BANK_RIGHT = 46,
    TILE_WATER_UP_R_BANK_BOTH = 47,
    TILE_WATER_LOW_L_BANK_BOTTOM = 48,
    TILE_WATER_LOW_L_BANK_LEFT = 49,
    TILE_WATER_LOW_L_BANK_BOTH = 50,
    TILE_WATER_LOW_R_BANK_BOTTOM = 51,
    TILE_WATER_LOW_R_BANK_RIGHT = 52,
    TILE_WATER_LOW_R_BANK_BOTH = 53,

    // --- Grass bank tiles (4) ---
    TILE_GRASS_BANK_TOP = 54,
    TILE_GRASS_BANK_BOTTOM = 55,
    TILE_GRASS_BANK_LEFT = 56,
    TILE_GRASS_BANK_RIGHT = 57
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
int tile_popcount(uint64_t mask);
TileType pick_weighted_tile(uint64_t possible_tiles);

#endif // TILES_H