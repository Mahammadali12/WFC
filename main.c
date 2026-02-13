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

Cell  grid [10][10]; // grid of 10x10 cells;


bool edges_compatible(EdgeType edge1, EdgeType edge2);
bool can_be_adjacent(TileType tile1, TileType tile2, Direction dir);


int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - logo raylib");

    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    Texture2D texture = LoadTexture("./profilePic.png");        // Texture loading

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // Draw
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawTexture(texture, screenWidth/2 - texture.width/2, screenHeight/2 - texture.height/2, WHITE);

            DrawText("this IS a texture!", 360, 370, 10, GRAY);

        EndDrawing();
    }

    // De-Initialization
    UnloadTexture(texture);       // Texture unloading

    CloseWindow();                // Close window and OpenGL context

    return 0;
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