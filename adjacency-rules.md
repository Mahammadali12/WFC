# Adjacency rules of tiles

## Abbreviations
- HR = horizontal line
- VR = vertical line
- low-R = lower right corner
- up-R = upper right corner
- low-L = lower left corner
- up-L = upper left corner
- empty = empty tile that does not have road

### Horizontal-line
- left = {HR,VR,low-L,up-L,empty}
- right = {HR,VR,low-R,up-R,empty}
- up = {HR,VR,low-L,low-R,empty}
- below = {HR,VR,up-L,up-R,empty}

### Vertical-line 
- left = {HR,VR,up-R,low-R,empty}
- right = {HR,VR,up-L,low-L,empty}
- up = {HR,VR,up-L,up-R,empty}
- below = {HR,VR,low-L,low-R,empty}

### Upper-Left-Corner
- left = {VR,up-R,low-R,empty}
- right = {HR,up-R,low-R}
- up = {HR,low-L,low-R,empty}
- below = {VR,low-L,low-R}

### Upper-Right-Corner
- left = {HR,up-L,low-L}
- right = {VR,up-L,low-L,empty}
- up = {HR,low-L,low-R,empty}
- below = {VR,low-L,low-R}

### Lower-Left-Corner
- left = {VR,low-R,up-R,empty}
- right = {HR,low-R,up-R}
- up = {VR,up-L,up-R}
- below = {HR,up-L,up-R,empty}

### Lower-Right-Corner
- left = {HR,low-L,up-L}
- right = {VR,up-L,low-L,empty}
- up = {VR,up-L,up-R}
- below = {HR,up-L,up-R,empty}

### empty
- left = {HR,VR,low-L,low-R,up-L,up-R,empty}
- right = {HR,VR,low-L,low-R,up-L,up-R,empty}
- up = {HR,VR,low-L,low-R,up-L,up-R,empty}
- below = {HR,VR,low-L,low-R,up-L,up-R,empty}


```c


typedef enum {
    EDGE_EMPTY = 0,
    EDGE_ROAD = 1
} EdgeType;

typedef struct {
    EdgeType top;
    EdgeType bottom;
    EdgeType left;
    EdgeType right;
} TileEdges;




typedef enum {
    TILE_HR = 0,
    TILE_VR = 1,
    TILE_UP_L = 2,
    TILE_UP_R = 3,
    TILE_LOW_L = 4,
    TILE_LOW_R = 5,
    TILE_EMPTY = 6
} TileType;


const TileEdges TILE_EDGES[7] = {
    [TILE_HR]    = {EDGE_EMPTY, EDGE_EMPTY, EDGE_ROAD, EDGE_ROAD},
    [TILE_VR]    = {EDGE_ROAD, EDGE_ROAD, EDGE_EMPTY, EDGE_EMPTY},
    [TILE_UP_L]  = {EDGE_EMPTY, EDGE_ROAD, EDGE_EMPTY, EDGE_ROAD},
    [TILE_UP_R]  = {EDGE_EMPTY, EDGE_ROAD, EDGE_ROAD, EDGE_EMPTY},
    [TILE_LOW_L] = {EDGE_ROAD, EDGE_EMPTY, EDGE_EMPTY, EDGE_ROAD},
    [TILE_LOW_R] = {EDGE_ROAD, EDGE_EMPTY, EDGE_ROAD, EDGE_EMPTY},
    [TILE_EMPTY] = {EDGE_EMPTY, EDGE_EMPTY, EDGE_EMPTY, EDGE_EMPTY}
};


typedef struct{
    int collapsed; // 0 uncollapsed, 1 collapsed
    TileType tile; // if collapsed which tile was chosen from [0-6]
    uint8_t possible_tiles; // set of possible tiles, maybe we can use 7 bit and each bit can represent each tile (I have chosen 7 type of tiles)
    int entropy;
}Cell;

Cell[10][10] grid; // grid of 10x10 cells;


typedef enum {
    DIR_TOP = 0,
    DIR_BOTTOM = 1,
    DIR_LEFT = 2,
    DIR_RIGHT = 3
} Direction;

bool edges_compatible(EdgeType edge1, EdgeType edge2) {
    // What's the rule for two edges touching each other?
    // Think about it.
    // Road can touch road or empty.
    // Empty can touch anything.
    // Figure out the logic yourself.
}

bool can_be_adjacent(TileType tile1, TileType tile2, Direction dir) {
    // tile1 is the reference tile
    // tile2 is in direction 'dir' from tile1
    
    // What edge of tile1 faces tile2?
    // What edge of tile2 faces tile1?
    
    // Example: if dir == DIR_RIGHT:
    //   tile1's RIGHT edge faces tile2's LEFT edge
    
    // Return whether those edges are compatible
}

```