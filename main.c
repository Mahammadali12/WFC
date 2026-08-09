#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <raylib.h>
#include "queue.h"

// gcc main.c queue.c -g -o bin/main -Wall -Wextra -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

#define GRID_SIZE 50
#define CELL_SIZE 28
#define MAX_SEEDS (GRID_SIZE * GRID_SIZE)

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

const char *TILE_NAMES[7] = {
    [TILE_HR]    = "Horizontal",
    [TILE_VR]    = "Vertical",
    [TILE_UP_L]  = "Upper-Left",
    [TILE_UP_R]  = "Upper-Right",
    [TILE_LOW_L] = "Lower-Left",
    [TILE_LOW_R] = "Lower-Right",
    [TILE_EMPTY] = "Empty"
};

typedef struct {
    int collapsed;          // 0 uncollapsed, 1 collapsed
    TileType tile;          // chosen tile once collapsed
    uint8_t possible_tiles; // bitmask: bit i set => tile i still possible
    int entropy;            // popcount of possible_tiles
} Cell;

Cell grid[GRID_SIZE][GRID_SIZE];

typedef struct {
    int x;
    int y;
    TileType tile;
} Seed;

Seed seeds[MAX_SEEDS];
int seed_count = 0;
TileType current_brush = TILE_HR;

const int screenWidth = GRID_SIZE * CELL_SIZE;
const int screenHeight = GRID_SIZE * CELL_SIZE;

Texture2D tile_textures[7];

typedef enum {
    STATE_DRAWING = 0,
    STATE_GENERATING
} AppState;

// Forward declarations
bool edges_compatible(EdgeType edge1, EdgeType edge2);
bool can_be_adjacent(TileType tile1, TileType tile2, Direction dir);
EdgeType get_edge(TileType tile, Direction direction);
Direction get_opposite_direction(Direction direction);
CellPos get_neighbor(CellPos pos, Direction dir);
void initialize_grid(void);
bool all_collapsed(void);
CellPos find_lowest_entropy_cell(void);
void collapse_cell(Cell *cell);
void propagate_cell(CellPos start);
void place_seed(int x, int y, TileType tile);
void paint_seed(int x, int y, TileType tile);
void erase_seed(int x, int y);
void clear_seeds(void);
void redraw_seeds(void);
void apply_seeds(void);
void update(void);
void draw_grid(void);
void draw_hud(int hover_x, int hover_y);
bool mouse_to_cell(int *out_x, int *out_y);
bool load_tile_textures(void);
void unload_tile_textures(void);

int main(void)
{
    InitWindow(screenWidth, screenHeight, "WFC road map generator");

    SetTargetFPS(60);
    srand((unsigned)time(NULL));

    if (!load_tile_textures())
    {
        CloseWindow();
        return 1;
    }

    clear_seeds();
    AppState state = STATE_DRAWING;

    while (!WindowShouldClose())
    {
        int hover_x = -1, hover_y = -1;
        mouse_to_cell(&hover_x, &hover_y);

        if (state == STATE_DRAWING)
        {
            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
                current_brush = (TileType)((current_brush + 1) % 7);

            if (hover_x >= 0)
            {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                    paint_seed(hover_x, hover_y, current_brush);
                if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON) || IsKeyDown(KEY_E))
                    erase_seed(hover_x, hover_y);
            }

            if (IsKeyPressed(KEY_R))
                clear_seeds();

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
            {
                apply_seeds();
                state = STATE_GENERATING;
            }
        }
        else if (state == STATE_GENERATING)
        {
            if (IsKeyPressed(KEY_R))
                apply_seeds();

            if (IsKeyPressed(KEY_M))
            {
                redraw_seeds();
                state = STATE_DRAWING;
            }

            if (IsKeyPressed(KEY_SPACE))
                update();

            if (!all_collapsed())
            {
                for (int i = 0; i < 20; i++)
                    update();
            }
        }

        BeginDrawing();

        draw_grid();

        if (state == STATE_DRAWING)
            draw_hud(hover_x, hover_y);

        EndDrawing();
    }

    unload_tile_textures();
    CloseWindow();

    return 0;
}

bool edges_compatible(EdgeType edge1, EdgeType edge2)
{
    return edge1 == edge2;
}

bool can_be_adjacent(TileType tile1, TileType tile2, Direction dir)
{
    return get_edge(tile1, dir) == get_edge(tile2, get_opposite_direction(dir));
}

CellPos get_neighbor(CellPos pos, Direction dir)
{
    switch (dir)
    {
        case DIR_TOP:    pos.y -= 1; break;
        case DIR_BOTTOM: pos.y += 1; break;
        case DIR_LEFT:   pos.x -= 1; break;
        case DIR_RIGHT:  pos.x += 1; break;
    }
    return pos;
}

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

bool load_tile_textures(void)
{
    tile_textures[TILE_HR]    = LoadTexture("tilesets/horizontal-line.png");
    tile_textures[TILE_VR]    = LoadTexture("tilesets/vertical-line.png");
    tile_textures[TILE_UP_L]  = LoadTexture("tilesets/upper-left-corner.png");
    tile_textures[TILE_UP_R]  = LoadTexture("tilesets/upper-right-corner.png");
    tile_textures[TILE_LOW_L] = LoadTexture("tilesets/lower-left-corner.png");
    tile_textures[TILE_LOW_R] = LoadTexture("tilesets/lower-right-corner.png");
    tile_textures[TILE_EMPTY] = LoadTexture("tilesets/empty-green.png");

    for (int i = 0; i < 7; i++)
    {
        if (tile_textures[i].id == 0)
        {
            fprintf(stderr, "Failed to load tile texture for TileType %d\n", i);
            return false;
        }
    }
    return true;
}

void unload_tile_textures(void)
{
    for (int i = 0; i < 7; i++)
        UnloadTexture(tile_textures[i]);
}

void initialize_grid(void)
{
    for (int x = 0; x < GRID_SIZE; x++)
    {
        for (int y = 0; y < GRID_SIZE; y++)
        {
            grid[x][y].collapsed = 0;
            grid[x][y].tile = TILE_EMPTY;
            grid[x][y].possible_tiles = 0b01111111;
            grid[x][y].entropy = 7;
        }
    }
}

void place_seed(int x, int y, TileType tile)
{
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE)
        return;
    grid[x][y].collapsed = 1;
    grid[x][y].tile = tile;
    grid[x][y].possible_tiles = (uint8_t)(1 << tile);
    grid[x][y].entropy = 0;
}

void paint_seed(int x, int y, TileType tile)
{
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE)
        return;

    if (grid[x][y].collapsed)
    {
        // Repaint existing seed with the new brush tile
        place_seed(x, y, tile);
        for (int i = 0; i < seed_count; i++)
        {
            if (seeds[i].x == x && seeds[i].y == y)
            {
                seeds[i].tile = tile;
                return;
            }
        }
        return;
    }

    place_seed(x, y, tile);
    seeds[seed_count].x = x;
    seeds[seed_count].y = y;
    seeds[seed_count].tile = tile;
    seed_count++;
}

void erase_seed(int x, int y)
{
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE)
        return;

    if (!grid[x][y].collapsed)
        return;

    grid[x][y].collapsed = 0;
    grid[x][y].tile = TILE_EMPTY;
    grid[x][y].possible_tiles = 0b01111111;
    grid[x][y].entropy = 7;

    for (int i = 0; i < seed_count; i++)
    {
        if (seeds[i].x == x && seeds[i].y == y)
        {
            seeds[i] = seeds[seed_count - 1];
            seed_count--;
            return;
        }
    }
}

void clear_seeds(void)
{
    initialize_grid();
    seed_count = 0;
}

void redraw_seeds(void)
{
    initialize_grid();
    for (int i = 0; i < seed_count; i++)
        place_seed(seeds[i].x, seeds[i].y, seeds[i].tile);
}

void apply_seeds(void)
{
    redraw_seeds();

    // Initialize constraints around every seed cell
    for (int x = 0; x < GRID_SIZE; x++)
        for (int y = 0; y < GRID_SIZE; y++)
            if (grid[x][y].collapsed)
                propagate_cell((CellPos){x, y});
}

bool all_collapsed(void)
{
    for (int x = 0; x < GRID_SIZE; x++)
        for (int y = 0; y < GRID_SIZE; y++)
            if (!grid[x][y].collapsed)
                return false;
    return true;
}

CellPos find_lowest_entropy_cell(void)
{
    int min_entropy = 8; // entropy of an uncollapsed cell is at most 7
    CellPos candidates[GRID_SIZE * GRID_SIZE];
    int candidate_count = 0;

    for (int x = 0; x < GRID_SIZE; x++)
    {
        for (int y = 0; y < GRID_SIZE; y++)
        {
            if (grid[x][y].collapsed)
                continue;

            int entropy = grid[x][y].entropy;

            if (entropy < min_entropy)
            {
                // New minimum found - DISCARD old candidates
                min_entropy = entropy;
                candidates[0] = (CellPos){x, y};
                candidate_count = 1;
            }
            else if (entropy == min_entropy)
            {
                // Tie - add to candidates
                candidates[candidate_count++] = (CellPos){x, y};
            }
        }
    }

    // Pick random from candidates (only called when not all cells collapsed)
    int idx = rand() % candidate_count;
    return candidates[idx];
}

void collapse_cell(Cell *cell)
{
    uint8_t possible = cell->possible_tiles;

    int available[7];
    int count = 0;
    for (int i = 0; i < 7; i++)
    {
        if (possible & (1 << i))
            available[count++] = i;
    }

    uint8_t chosen = (uint8_t)available[rand() % count];

    cell->collapsed = 1;
    cell->entropy = 0;
    cell->tile = (TileType)chosen;
    cell->possible_tiles = (uint8_t)(1 << chosen);
}

void propagate_cell(CellPos start)
{
    Queue q;
    if (!queue_init(&q, GRID_SIZE * GRID_SIZE * 4))
        return;

    queue_enqueue(&q, start);

    while (!queue_is_empty(&q))
    {
        CellPos current;
        queue_dequeue(&q, &current);

        Cell *cell = &grid[current.x][current.y];
        if (!cell->collapsed)
            continue;

        for (int d = 0; d < 4; d++)
        {
            Direction dir = (Direction)d;
            CellPos neighbor_pos = get_neighbor(current, dir);

            if (neighbor_pos.x < 0 || neighbor_pos.x >= GRID_SIZE ||
                neighbor_pos.y < 0 || neighbor_pos.y >= GRID_SIZE)
                continue;

            Cell *neighbor = &grid[neighbor_pos.x][neighbor_pos.y];
            if (neighbor->collapsed)
                continue;

            // Keep only neighbor tiles whose facing edge matches this cell's edge
            uint8_t new_possible = 0;
            for (int t = 0; t < 7; t++)
            {
                if (neighbor->possible_tiles & (1 << t))
                {
                    if (can_be_adjacent(cell->tile, (TileType)t, dir))
                        new_possible |= (uint8_t)(1 << t);
                }
            }

            if (new_possible == neighbor->possible_tiles)
                continue;

            neighbor->possible_tiles = new_possible;

            int entropy = 0;
            for (int t = 0; t < 7; t++)
                if (new_possible & (1 << t))
                    entropy++;
            neighbor->entropy = entropy;

            queue_enqueue(&q, neighbor_pos);
        }
    }

    queue_free(&q);
}

void update(void)
{
    if (all_collapsed())
        return;

    CellPos min_cell = find_lowest_entropy_cell();

    if (grid[min_cell.x][min_cell.y].entropy == 0)
    {
        // Contradiction - no tile fits, restart with the same drawn seeds
        apply_seeds();
        return;
    }

    collapse_cell(&grid[min_cell.x][min_cell.y]);
    propagate_cell(min_cell);
}

bool mouse_to_cell(int *out_x, int *out_y)
{
    Vector2 m = GetMousePosition();
    int x = (int)(m.x / CELL_SIZE);
    int y = (int)(m.y / CELL_SIZE);

    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE)
        return false;

    *out_x = x;
    *out_y = y;
    return true;
}

void draw_grid(void)
{
    ClearBackground(RAYWHITE);

    for (int x = 0; x < GRID_SIZE; x++)
    {
        for (int y = 0; y < GRID_SIZE; y++)
        {
            Rectangle dest = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};

            if (grid[x][y].collapsed)
            {
                DrawTexturePro(tile_textures[grid[x][y].tile],
                               (Rectangle){0, 0, tile_textures[grid[x][y].tile].width, tile_textures[grid[x][y].tile].height},
                               dest, (Vector2){0, 0}, 0, WHITE);
            }
            else
            {
                DrawTexturePro(tile_textures[TILE_EMPTY],
                               (Rectangle){0, 0, tile_textures[TILE_EMPTY].width, tile_textures[TILE_EMPTY].height},
                               dest, (Vector2){0, 0}, 0, (Color){255, 255, 255, 45});
            }
        }
    }
}

void draw_hud(int hover_x, int hover_y)
{
    // Hovered cell highlight
    if (hover_x >= 0)
        DrawRectangleLines(hover_x * CELL_SIZE, hover_y * CELL_SIZE, CELL_SIZE, CELL_SIZE, RED);

    // Brush indicator panel (top-left overlay)
    DrawRectangle(8, 8, 220, 96, (Color){0, 0, 0, 140});

    Rectangle icon = {20, 16, 40, 40};
    DrawTexturePro(tile_textures[current_brush],
                   (Rectangle){0, 0, tile_textures[current_brush].width, tile_textures[current_brush].height},
                   icon, (Vector2){0, 0}, 0, WHITE);
    DrawRectangleLines((int)icon.x, (int)icon.y, (int)icon.width, (int)icon.height, RED);

    char brush_line[64];
    snprintf(brush_line, sizeof(brush_line), "Brush: %s", TILE_NAMES[current_brush]);
    DrawText(brush_line, 70, 20, 20, WHITE);

    char seed_line[64];
    snprintf(seed_line, sizeof(seed_line), "Seeds: %d", seed_count);
    DrawText(seed_line, 70, 46, 20, WHITE);

    DrawText("RMB: cycle tile", 70, 72, 16, LIGHTGRAY);

    // Bottom hint bar
    DrawRectangle(8, screenHeight - 32, screenWidth - 16, 24, (Color){0, 0, 0, 140});
    DrawText("LMB: paint | RMB: cycle | MMB or E: erase | R: clear | Enter: generate",
             16, screenHeight - 28, 16, WHITE);
}
