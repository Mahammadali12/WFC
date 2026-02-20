#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <stdint.h>
// gcc main.c -g -o main -Wall -Wextra -lraylib -lGL -lm -lpthread -ldl -lrt -lX11


typedef enum {
    DIR_TOP = 0,
    DIR_BOTTOM = 1,
    DIR_LEFT = 2,
    DIR_RIGHT = 3
} Direction;


typedef enum {
    EDGE_EMPTY = 0,
    EDGE_ROAD = 1
} EdgeType;


typedef enum {
    TILE_HR = 0,
    TILE_VR = 1,
    TILE_UP_L = 2,
    TILE_UP_R = 3,
    TILE_LOW_L = 4,
    TILE_LOW_R = 5,
    TILE_EMPTY = 6
} TileType;

typedef struct {
    EdgeType top;
    EdgeType bottom;
    EdgeType left;
    EdgeType right;
} TileEdges;

typedef struct {
    int x,y;
}CellPos;



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

Cell grid [100][100]; // grid of 100x100 cells;


bool edges_compatible(EdgeType edge1, EdgeType edge2);
bool can_be_adjacent(TileType tile1, TileType tile2, Direction dir);
EdgeType get_edge(TileType tile, Direction direction);
Direction get_opposite_direction(Direction direction);
void initialize_grid();

const int screenWidth = 900;
const int screenHeight = 600;

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - logo raylib");

    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    Texture2D texture = LoadTexture("./tilesets/empty-green.png");// Texture loading

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {


        //INIT GRID
        //UPDATE()



        //* Draw
        //* BeginDrawing();
        // *    ClearBackground(RAYWHITE);
        // *    DrawTexture(texture, screenWidth/2 - texture.width/2, screenHeight/2 - texture.height/2, WHITE);
        // *EndDrawing();
    }

    // De-Initialization
    UnloadTexture(texture);       // Texture unloading

    CloseWindow();                // Close window and OpenGL context

    return 0;
}

void update()
{
    // while ()  not all cells are collapsed 
        //    {
        //      min_cell = find_lowest_entropy()
        //      
        //      if( min_cell has zero entropy)
        //          start again :naive restart
        //
        //      collapse_cell(min_cell);
        //      
        //      propagate_cell(min_cell);
        //      
        //      display_grid();
        //      
        //      
        //
        //    }

}

bool edges_compatible(EdgeType edge1, EdgeType edge2) 
{
    return edge1 == edge2;
}

bool can_be_adjacent(TileType tile1, TileType tile2, Direction dir) 
{
    return get_edge(tile1,dir) == get_edge(tile2,get_opposite_direction(dir));
}

EdgeType get_edge(TileType tile, Direction direction)
{
    switch(direction) {
        case DIR_TOP: return TILE_EDGES[tile].top;
        case DIR_BOTTOM: return TILE_EDGES[tile].bottom;
        case DIR_LEFT: return TILE_EDGES[tile].left;
        case DIR_RIGHT: return TILE_EDGES[tile].right;
    }
    return EDGE_EMPTY; // fallback, should never hit
}

Direction get_opposite_direction(Direction direction)
{
    if(direction == DIR_BOTTOM)
    return DIR_TOP;

    if(direction == DIR_TOP)
    return DIR_BOTTOM;

    if(direction == DIR_LEFT)
    return DIR_RIGHT;

    if(direction == DIR_RIGHT)
    return DIR_LEFT;
}

void initialize_grid()
{
    for (int i = 0; i < 100; i++) //! HARDCODED value
    {
        for (int j = 0; j < 100; j++) //! HARDCODED value
        {
            grid[i][j].collapsed = 0;
            grid[i][j].tile = -1;
            grid[i][j].possible_tiles = 0b01111111;
            grid[i][j].entropy = 7;
        }   
    }
}


CellPos find_lowest_entropy_cell(Cell grid[100][100]) {
    int min_entropy = INT32_MAX;
    CellPos candidates[100*100];  // max possible candidates
    int candidate_count = 0;
    
    for (int x = 0; x < 100; x++) {
        for (int y = 0; y < 100; y++) {
            if (grid[x][y].collapsed)
                continue;
            
            int entropy = grid[x][y].entropy;
            
            if (entropy < min_entropy) {
                // New minimum found - DISCARD old candidates
                min_entropy = entropy;
                candidates[0] = (CellPos){x, y};
                candidate_count = 1;  // reset count
            }
            else if (entropy == min_entropy) {
                // Tie - add to candidates
                candidates[candidate_count++] = (CellPos){x, y};
            }
        }
    }
    
    // Pick random from candidates
    int idx = rand() % candidate_count;
    return candidates[idx];
}