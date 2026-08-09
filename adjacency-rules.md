# Adjacency rules of tiles

The generator uses **edge-based, strict-equality adjacency**: a tile may be placed
next to another only if the edge they share is the same type on both sides.

A tile facing a direction `dir` exposes an edge; the neighbor facing the opposite
direction must expose the same edge type (`EDGE_ROAD` matches `EDGE_ROAD`,
`EDGE_EMPTY` matches `EDGE_EMPTY`). This is implemented by `can_be_adjacent()`
in `tiles.c`:

```c
bool can_be_adjacent(TileType tile1, TileType tile2, Direction dir)
{
    return get_edge(tile1, dir) == get_edge(tile2, get_opposite_direction(dir));
}
```

## Edge types

```c
typedef enum { EDGE_EMPTY = 0, EDGE_ROAD = 1 } EdgeType;
```

## Tile types (12 total)

| Index | Enum        | Top    | Bottom | Left   | Right  |
|-------|-------------|--------|--------|--------|--------|
| 0     | TILE_HR     | empty  | empty  | road   | road   |
| 1     | TILE_VR     | road   | road   | empty  | empty  |
| 2     | TILE_UP_L   | empty  | road   | empty  | road   |
| 3     | TILE_UP_R   | empty  | road   | road   | empty  |
| 4     | TILE_LOW_L  | road   | empty  | empty  | road   |
| 5     | TILE_LOW_R  | road   | empty  | road   | empty  |
| 6     | TILE_EMPTY  | empty  | empty  | empty  | empty  |
| 7     | TILE_CROSS  | road   | road   | road   | road   |
| 8     | TILE_T_UP   | road   | empty  | road   | road   |
| 9     | TILE_T_DOWN | empty  | road   | road   | road   |
| 10    | TILE_T_LEFT | road   | road   | road   | empty  |
| 11    | TILE_T_RIGHT| road   | road   | empty  | road   |

`TILE_CROSS` and the four `TILE_T_*` junctions mean **roads can now
intersect**: the crossing connects all four edges, and each T-junction connects
three. Before these tiles existed, no tile had 3–4 road edges, so a road could
never meet another road except by passing straight through it.

## Derived rules

- Roads connect along their `EDGE_ROAD` edges; any two road edges can meet,
  regardless of the tile type.
- Empty edges only touch empty edges. A road edge never touches an empty edge.
- The strict rule means the compatibility check never "loosens": there is no
  case where `EDGE_ROAD` is allowed to face `EDGE_EMPTY`.

## Weights

`TILE_WEIGHTS[]` (also in `tiles.c`) biases the collapse so maps read as road
networks rather than noise:

| Tile         | Weight |
|--------------|--------|
| HR / VR      | 10     |
| EMPTY        | 12     |
| corners      | 3      |
| cross / T-*  | 2      |

`pick_weighted_tile()` selects among a cell's `possible_tiles` mask weighted by
these values.
