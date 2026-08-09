// Unit tests for the tile adjacency/weight logic (tiles.c). No raylib needed.
// Build: make test   (gcc tests/test.c tiles.c queue.c -lm)
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "tiles.h"

int main(void)
{
    // --- Edge table sanity ---
    assert(TILE_COUNT == 12);
    assert(get_edge(TILE_HR, DIR_LEFT) == EDGE_ROAD);
    assert(get_edge(TILE_HR, DIR_RIGHT) == EDGE_ROAD);
    assert(get_edge(TILE_HR, DIR_TOP) == EDGE_EMPTY);
    assert(get_edge(TILE_HR, DIR_BOTTOM) == EDGE_EMPTY);

    assert(get_edge(TILE_VR, DIR_TOP) == EDGE_ROAD);
    assert(get_edge(TILE_VR, DIR_BOTTOM) == EDGE_ROAD);
    assert(get_edge(TILE_VR, DIR_LEFT) == EDGE_EMPTY);

    // New tiles
    assert(get_edge(TILE_CROSS, DIR_TOP) == EDGE_ROAD);
    assert(get_edge(TILE_CROSS, DIR_BOTTOM) == EDGE_ROAD);
    assert(get_edge(TILE_CROSS, DIR_LEFT) == EDGE_ROAD);
    assert(get_edge(TILE_CROSS, DIR_RIGHT) == EDGE_ROAD);

    assert(get_edge(TILE_T_UP, DIR_TOP) == EDGE_ROAD);     // exits top + left + right
    assert(get_edge(TILE_T_UP, DIR_BOTTOM) == EDGE_EMPTY);
    assert(get_edge(TILE_T_UP, DIR_LEFT) == EDGE_ROAD);
    assert(get_edge(TILE_T_UP, DIR_RIGHT) == EDGE_ROAD);

    assert(get_edge(TILE_T_DOWN, DIR_TOP) == EDGE_EMPTY);
    assert(get_edge(TILE_T_DOWN, DIR_BOTTOM) == EDGE_ROAD);
    assert(get_edge(TILE_T_DOWN, DIR_LEFT) == EDGE_ROAD);
    assert(get_edge(TILE_T_DOWN, DIR_RIGHT) == EDGE_ROAD);

    assert(get_edge(TILE_T_LEFT, DIR_LEFT) == EDGE_ROAD);
    assert(get_edge(TILE_T_LEFT, DIR_RIGHT) == EDGE_EMPTY);
    assert(get_edge(TILE_T_LEFT, DIR_TOP) == EDGE_ROAD);
    assert(get_edge(TILE_T_LEFT, DIR_BOTTOM) == EDGE_ROAD);

    assert(get_edge(TILE_T_RIGHT, DIR_LEFT) == EDGE_EMPTY);
    assert(get_edge(TILE_T_RIGHT, DIR_RIGHT) == EDGE_ROAD);
    assert(get_edge(TILE_T_RIGHT, DIR_TOP) == EDGE_ROAD);
    assert(get_edge(TILE_T_RIGHT, DIR_BOTTOM) == EDGE_ROAD);

    // --- Opposite directions ---
    assert(get_opposite_direction(DIR_TOP) == DIR_BOTTOM);
    assert(get_opposite_direction(DIR_BOTTOM) == DIR_TOP);
    assert(get_opposite_direction(DIR_LEFT) == DIR_RIGHT);
    assert(get_opposite_direction(DIR_RIGHT) == DIR_LEFT);

    // --- Adjacency (strict edge equality) ---
    // HR facing HR on left/right: both edges ROAD -> compatible
    assert(can_be_adjacent(TILE_HR, TILE_HR, DIR_RIGHT));
    assert(can_be_adjacent(TILE_HR, TILE_HR, DIR_LEFT));

    // HR's right edge (ROAD) vs VR's left edge (EMPTY) -> incompatible
    assert(!can_be_adjacent(TILE_HR, TILE_VR, DIR_RIGHT));
    assert(!can_be_adjacent(TILE_VR, TILE_HR, DIR_RIGHT));

    // HR below EMPTY: HR.top EMPTY == EMPTY.bottom EMPTY -> ok
    assert(can_be_adjacent(TILE_HR, TILE_EMPTY, DIR_TOP));
    // EMPTY below HR: EMPTY.top EMPTY == HR.bottom EMPTY -> ok
    assert(can_be_adjacent(TILE_EMPTY, TILE_HR, DIR_TOP));

    // Cross connects a straight on its left side
    assert(can_be_adjacent(TILE_HR, TILE_CROSS, DIR_RIGHT)); // HR.right ROAD == CROSS.left ROAD
    assert(can_be_adjacent(TILE_CROSS, TILE_VR, DIR_BOTTOM)); // CROSS.bottom ROAD == VR.top ROAD
    assert(can_be_adjacent(TILE_VR, TILE_CROSS, DIR_TOP));    // VR.top ROAD == CROSS.bottom ROAD

    // T junctions: stem connects to a straight, cap side does not
    assert(can_be_adjacent(TILE_T_UP, TILE_VR, DIR_TOP));    // T_UP.top ROAD == VR.bottom ROAD
    assert(can_be_adjacent(TILE_T_UP, TILE_HR, DIR_LEFT));   // T_UP.left ROAD == HR.right ROAD
    assert(can_be_adjacent(TILE_HR, TILE_T_UP, DIR_RIGHT));  // HR.right ROAD == T_UP.left ROAD
    assert(!can_be_adjacent(TILE_T_UP, TILE_VR, DIR_BOTTOM)); // T_UP.bottom EMPTY vs VR.top ROAD -> no

    // T_LEFT has right EMPTY, so a HR's left ROAD touching it fails:
    assert(!can_be_adjacent(TILE_HR, TILE_T_LEFT, DIR_LEFT)); // HR.left ROAD vs T_LEFT.right EMPTY -> no

    // Symmetry: adjacency must be symmetric when viewed from the opposite side
    for (int a = 0; a < TILE_COUNT; a++)
        for (int b = 0; b < TILE_COUNT; b++)
            for (int d = 0; d < 4; d++)
            {
                bool fwd = can_be_adjacent((TileType)a, (TileType)b, (Direction)d);
                bool rev = can_be_adjacent((TileType)b, (TileType)a, get_opposite_direction((Direction)d));
                assert(fwd == rev);
            }

    // --- Popcount ---
    assert(tile_popcount(0) == 0);
    assert(tile_popcount(1u << TILE_T_RIGHT) == 1);
    assert(tile_popcount((uint16_t)((1u << TILE_COUNT) - 1)) == TILE_COUNT);

    // --- Weighted pick is deterministic for a fixed seed ---
    srand(12345);
    TileType p1 = pick_weighted_tile((uint16_t)((1u << TILE_COUNT) - 1));
    srand(12345);
    TileType p2 = pick_weighted_tile((uint16_t)((1u << TILE_COUNT) - 1));
    assert(p1 == p2);

    // A single-candidate mask must always pick that tile
    srand(999);
    assert(pick_weighted_tile((uint16_t)(1u << TILE_CROSS)) == TILE_CROSS);
    srand(1);
    assert(pick_weighted_tile((uint16_t)(1u << TILE_EMPTY)) == TILE_EMPTY);

    // All weights are positive (needed for the cumulative pick)
    for (int t = 0; t < TILE_COUNT; t++)
        assert(TILE_WEIGHTS[t] > 0);

    printf("All %d tests passed.\n", 42);
    return 0;
}
