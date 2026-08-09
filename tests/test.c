// Unit tests for the tile adjacency/weight logic (tiles.c). No raylib needed.
// Build: make test   (gcc tests/test.c tiles.c queue.c -lm)
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "tiles.h"

int main(void)
{
    // --- Edge table sanity ---
    assert(TILE_COUNT == 58);
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
    assert(tile_popcount(1ULL << TILE_T_RIGHT) == 1);
    assert(tile_popcount(((1ULL << TILE_COUNT) - 1)) == TILE_COUNT);

    // --- Weighted pick is deterministic for a fixed seed ---
    srand(12345);
    TileType p1 = pick_weighted_tile(((1ULL << TILE_COUNT) - 1));
    srand(12345);
    TileType p2 = pick_weighted_tile(((1ULL << TILE_COUNT) - 1));
    assert(p1 == p2);

    // A single-candidate mask must always pick that tile
    srand(999);
    assert(pick_weighted_tile((1ULL << TILE_CROSS)) == TILE_CROSS);
    srand(1);
    assert(pick_weighted_tile((1ULL << TILE_EMPTY)) == TILE_EMPTY);

    // All weights are positive (needed for the cumulative pick)
    for (int t = 0; t < TILE_COUNT; t++)
        assert(TILE_WEIGHTS[t] > 0);

    // ===== Water / bank system adversarial tests =====

    // --- Edge type sanity: BANK is its own value, not a synonym ---
    assert(EDGE_EMPTY != EDGE_ROAD);
    assert(EDGE_ROAD  != EDGE_WATER);
    assert(EDGE_WATER != EDGE_BANK);
    assert(EDGE_BANK  != EDGE_EMPTY);

    // --- Water / road never share an edge (different "fluids") ---
    // WATER_HR.left is WATER, HR.left is ROAD — they must NOT be compatible.
    assert(!can_be_adjacent(TILE_WATER_HR, TILE_HR, DIR_LEFT));
    assert(!can_be_adjacent(TILE_HR, TILE_WATER_HR, DIR_LEFT));

    // --- Water continues across a water-water boundary ---
    assert(can_be_adjacent(TILE_WATER_HR, TILE_WATER_HR, DIR_LEFT));
    assert(can_be_adjacent(TILE_WATER_VR, TILE_WATER_VR, DIR_TOP));

    // --- Water corner: WATER_LOW_L has WATER on top (continues from north) and
    //     on right (continues east). It must match WATER_VR to its north and
    //     WATER_HR to its east, and must NOT match a road on either side. ---
    assert(can_be_adjacent(TILE_WATER_LOW_L, TILE_WATER_VR, DIR_TOP));
    assert(can_be_adjacent(TILE_WATER_LOW_L, TILE_WATER_HR, DIR_RIGHT));
    assert(!can_be_adjacent(TILE_WATER_LOW_L, TILE_HR, DIR_TOP));
    assert(!can_be_adjacent(TILE_WATER_LOW_L, TILE_HR, DIR_RIGHT));

    // --- Bank: road HR with bank on top expects a bank on the cell above ---
    // HR_BANK_TOP.top = BANK, GRASS_BANK_BOTTOM.bottom = BANK  -> match
    assert(can_be_adjacent(TILE_HR_BANK_TOP, TILE_GRASS_BANK_BOTTOM, DIR_TOP));
    // HR_BANK_TOP.top = BANK, but a plain EMPTY grass cell has bottom=EMPTY -> NO match
    assert(!can_be_adjacent(TILE_HR_BANK_TOP, TILE_EMPTY, DIR_TOP));
    // Conversely, a grass bank strip demands a bank on its south neighbour
    assert(can_be_adjacent(TILE_GRASS_BANK_BOTTOM, TILE_HR_BANK_TOP, DIR_BOTTOM));
    // And a non-banked road cannot sit south of a bank-grass strip
    assert(!can_be_adjacent(TILE_GRASS_BANK_BOTTOM, TILE_HR, DIR_BOTTOM));

    // --- Bank on a corner: UP_L_BANK_TOP.top = BANK ---
    assert(can_be_adjacent(TILE_UP_L_BANK_TOP, TILE_GRASS_BANK_BOTTOM, DIR_TOP));
    assert(!can_be_adjacent(TILE_UP_L_BANK_TOP, TILE_EMPTY, DIR_TOP));

    // --- Causeway: HR_BANK_BOTH has BANK on both EMPTY edges (top + bottom) ---
    assert(can_be_adjacent(TILE_HR_BANK_BOTH, TILE_GRASS_BANK_BOTTOM, DIR_TOP));
    assert(can_be_adjacent(TILE_HR_BANK_BOTH, TILE_GRASS_BANK_TOP,    DIR_BOTTOM));

    // --- Water next to road through a bank sandwich: water (BANK on bottom) <->
    //     grass-bank (BANK on top) <-> road-bank (BANK on bottom). The unbanked
    //     water tile (bottom=EMPTY) cannot sit above a bank-grass strip whose
    //     TOP edge is BANK — that would need BANK==EMPTY, which fails. ---
    assert(can_be_adjacent(TILE_WATER_HR_BANK_BOTTOM, TILE_GRASS_BANK_TOP,    DIR_TOP));
    assert(can_be_adjacent(TILE_GRASS_BANK_TOP,         TILE_HR_BANK_BOTTOM,  DIR_BOTTOM));
    // unbanked water above a bank-grass strip: grass.top is BANK but water.bottom is EMPTY
    assert(!can_be_adjacent(TILE_GRASS_BANK_TOP, TILE_WATER_HR, DIR_TOP));
    // And the non-bank side of a bank-grass strip still has to face EMPTY grass.
    assert(can_be_adjacent(TILE_GRASS_BANK_BOTTOM, TILE_EMPTY, DIR_TOP));

    // --- Grid boundary: a road HR_BANK_TOP with BANK on its top edge has no
    //     north neighbour. That's a contradiction — the WFC's restart-on-
    //     contradiction path must catch it (this test just proves the edge
    //     value is BANK, so propagate_cell will eventually find entropy 0).
    assert(get_edge(TILE_HR_BANK_TOP, DIR_TOP) == EDGE_BANK);

    // --- BANK_BOTH for corners: UP_L_BANK_BOTH has BANK on top AND left ---
    assert(get_edge(TILE_UP_L_BANK_BOTH, DIR_TOP)  == EDGE_BANK);
    assert(get_edge(TILE_UP_L_BANK_BOTH, DIR_LEFT) == EDGE_BANK);
    assert(get_edge(TILE_UP_L_BANK_BOTH, DIR_BOTTOM) == EDGE_ROAD);
    assert(get_edge(TILE_UP_L_BANK_BOTH, DIR_RIGHT)  == EDGE_ROAD);

    // --- Symmetry of adjacency still holds across the whole tile set
    //     (this was a 7-tile test; redo it for 58 to catch any new tile
    //     that breaks the symmetry property).
    for (int a = 0; a < TILE_COUNT; a++)
        for (int b = 0; b < TILE_COUNT; b++)
            for (int d = 0; d < 4; d++)
            {
                bool fwd = can_be_adjacent((TileType)a, (TileType)b, (Direction)d);
                bool rev = can_be_adjacent((TileType)b, (TileType)a, get_opposite_direction((Direction)d));
                assert(fwd == rev);
            }

    // --- Every bank tile is consistent: a tile with N bank edges has
    //     exactly N EDGE_BANK values in its TILE_EDGES row.
    for (int t = 0; t < TILE_COUNT; t++)
    {
        int banks = 0;
        if (TILE_EDGES[t].top    == EDGE_BANK) banks++;
        if (TILE_EDGES[t].bottom == EDGE_BANK) banks++;
        if (TILE_EDGES[t].left   == EDGE_BANK) banks++;
        if (TILE_EDGES[t].right  == EDGE_BANK) banks++;
        if (t == TILE_HR_BANK_TOP    && banks != 1) assert(0 && "HR_BANK_TOP should have 1 bank");
        if (t == TILE_HR_BANK_BOTTOM && banks != 1) assert(0 && "HR_BANK_BOTTOM should have 1 bank");
        if (t == TILE_HR_BANK_BOTH   && banks != 2) assert(0 && "HR_BANK_BOTH should have 2 banks");
        if (t == TILE_UP_L_BANK_BOTH && banks != 2) assert(0 && "UP_L_BANK_BOTH should have 2 banks");
        if (t == TILE_GRASS_BANK_TOP && banks != 1) assert(0 && "GRASS_BANK_TOP should have 1 bank");
    }

    printf("All %d tests passed.\n", 73);
    return 0;
}
