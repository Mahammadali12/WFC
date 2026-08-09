# Adjacency rules of tiles

The generator uses **edge-based, strict-equality adjacency**: a tile may be placed
next to another only if the edge they share is the same type on both sides.

A tile facing a direction `dir` exposes an edge; the neighbor facing the opposite
direction must expose the same edge type (`EDGE_ROAD` matches `EDGE_ROAD`,
`EDGE_EMPTY` matches `EDGE_EMPTY`). This is implemented by `can_be_adjacent()`
in `src/tiles.c`:

```c
bool can_be_adjacent(TileType tile1, TileType tile2, Direction dir)
{
    return get_edge(tile1, dir) == get_edge(tile2, get_opposite_direction(dir));
}
```

## Edge types

```c
typedef enum { EDGE_EMPTY = 0, EDGE_ROAD = 1, EDGE_WATER = 2, EDGE_BANK = 3 } EdgeType;
```

## Tile types (58 total)

See `docs/tile-taxonomy.md` for the full 58-tile table. The original 12-tile
set (7 road base + 5 crossings/T-junctions) has been expanded with water,
bank, and grass-bank variants.

## Derived rules

- Roads connect along their `EDGE_ROAD` edges; any two road edges can meet,
  regardless of the tile type.
- Empty edges only touch empty edges. A road edge never touches an empty edge.
- Water edges only touch water edges.
- Bank edges only touch bank edges.
- The strict rule means the compatibility check never "loosens": there is no
  case where different edge types are allowed to face each other.

## Weights

`TILE_WEIGHTS[]` (in `src/tiles.c`) biases the collapse so maps read as road
networks rather than noise. See the source for the full 58-entry table.
`pick_weighted_tile()` selects among a cell's `possible_tiles` mask weighted by
these values.
