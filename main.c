#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#if !defined(__EMSCRIPTEN__)
#include <sys/stat.h>
#endif
#include <raylib.h>
#include "queue.h"
#include "tiles.h"

// Emscripten web helpers
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>

EM_JS(int, wfc_local_get, (const char *key, char *out, int max),
{
    var k = UTF8ToString(key);
    var v = localStorage.getItem(k);
    if (v === null || v.length === 0) return 0;
    if (v.length >= max) v = v.substring(0, max - 1);
    stringToUTF8(v, out, max);
    return 1;
});

EM_JS(void, wfc_local_set, (const char *key, const char *val),
{
    localStorage.setItem(UTF8ToString(key), UTF8ToString(val));
});

EM_JS(int, wfc_url_get, (char *out, int max),
{
    var s = window.location.search || "";
    if (!s) return 0;
    s = decodeURIComponent(s);
    if (s.length >= max) s = s.substring(0, max - 1);
    stringToUTF8(s, out, max);
    return 1;
});

EM_JS(void, wfc_download, (void *ptr, int size, const char *name),
{
    var bytes = new Uint8Array(HEAPU8.buffer, ptr, size);
    var blob = new Blob([bytes], { type: "application/octet-stream" });
    var a = document.createElement("a");
    a.href = URL.createObjectURL(blob);
    a.download = UTF8ToString(name);
    a.click();
    setTimeout(function () { URL.revokeObjectURL(a.href); }, 2000);
});
#endif

#define GRID_SIZE 50
#define CELL_SIZE 28
#define MAX_SEEDS (GRID_SIZE * GRID_SIZE)
#define UNDO_DEPTH 64
#define MAX_RESTARTS 50

const char *TILE_TEXTURE_FILES[TILE_COUNT] = {
    [TILE_HR]      = "tilesets/horizontal-line.png",
    [TILE_VR]      = "tilesets/vertical-line.png",
    [TILE_UP_L]    = "tilesets/upper-left-corner.png",
    [TILE_UP_R]    = "tilesets/upper-right-corner.png",
    [TILE_LOW_L]   = "tilesets/lower-left-corner.png",
    [TILE_LOW_R]   = "tilesets/lower-right-corner.png",
    [TILE_EMPTY]   = "tilesets/empty-green.png",
    [TILE_CROSS]   = "tilesets/crossing.png",
    [TILE_T_UP]    = "tilesets/t-up.png",
    [TILE_T_DOWN]  = "tilesets/t-down.png",
    [TILE_T_LEFT]  = "tilesets/t-left.png",
    [TILE_T_RIGHT] = "tilesets/t-right.png",
};

typedef struct {
    int collapsed;
    TileType tile;
    uint32_t possible_tiles;
    int entropy;
} Cell;

Cell grid[GRID_SIZE][GRID_SIZE];
uint8_t cell_flash[GRID_SIZE][GRID_SIZE];

typedef struct {
    int x;
    int y;
    TileType tile;
} Seed;

typedef struct {
    Seed *data;
    int len;
} SeedSnapshot;

Seed seeds[MAX_SEEDS];
int seed_count = 0;
int current_brush = TILE_HR;

const int screenWidth = GRID_SIZE * CELL_SIZE;
const int screenHeight = GRID_SIZE * CELL_SIZE;

Texture2D tile_textures[TILE_COUNT];

typedef enum {
    STATE_DRAWING = 0,
    STATE_GENERATING
} AppState;

AppState state = STATE_DRAWING;

unsigned int wfc_seed = 0;
int cells_per_frame = 20;
int gen_attempts = 0;
int consecutive_restarts = 0;
int collapsed_count = 0;

char notice[160];
int notice_ticks = 0;

SeedSnapshot undo_stack[UNDO_DEPTH];
SeedSnapshot redo_stack[UNDO_DEPTH];
int undo_top = 0;
int redo_top = 0;
bool undo_suppress = false;

// --- Forward declarations ---
CellPos get_neighbor(CellPos pos, Direction dir);
void initialize_grid(void);
bool all_collapsed(void);
CellPos find_lowest_entropy_cell(void);
void collapse_cell(int x, int y);
void propagate_cell(CellPos start);
void place_seed(int x, int y, TileType tile);
void paint_seed(int x, int y, TileType tile);
void erase_seed(int x, int y);
void clear_seeds(void);
void redraw_seeds(void);
void apply_seeds(void);
void update(void);
void begin_generation(void);
bool mouse_to_cell(int *out_x, int *out_y);
bool load_tile_textures(void);
void unload_tile_textures(void);
void draw_grid(int hover_x, int hover_y);
void draw_hud(int hover_x, int hover_y);
void handle_drawing_input(int *hover_x, int *hover_y);
void handle_generating_input(void);
void export_map(void);
void parse_seed_string(const char *text);
void handle_url_seeds(void);
bool save_pattern(int slot);
bool load_pattern(int slot);
void undo_one(void);
void redo_one(void);

// --- Snapshot helpers ---

static void snapshots_push(SeedSnapshot *stack, int *top)
{
    if (*top > 0)
    {
        const SeedSnapshot *last = &stack[*top - 1];
        if (last->len == seed_count &&
            (seed_count == 0 || memcmp(last->data, seeds, (size_t)seed_count * sizeof(Seed)) == 0))
            return;
    }

    if (*top >= UNDO_DEPTH)
    {
        if (stack[0].data) free(stack[0].data);
        memmove(&stack[0], &stack[1], (UNDO_DEPTH - 1) * sizeof(SeedSnapshot));
        *top = UNDO_DEPTH - 1;
    }

    stack[*top].len = seed_count;
    stack[*top].data = NULL;
    if (seed_count > 0)
    {
        stack[*top].data = (Seed *)malloc((size_t)seed_count * sizeof(Seed));
        if (stack[*top].data)
            memcpy(stack[*top].data, seeds, (size_t)seed_count * sizeof(Seed));
    }
    (*top)++;
}

static bool snapshots_pop(SeedSnapshot *stack, int *top)
{
    if (*top == 0) return false;
    SeedSnapshot s = stack[--(*top)];
    if (s.data && s.len > 0)
    {
        memcpy(seeds, s.data, (size_t)s.len * sizeof(Seed));
        seed_count = s.len;
    }
    else
    {
        seed_count = 0;
    }
    free(s.data);
    redraw_seeds();
    return true;
}

static void push_undo(void)
{
    snapshots_push(undo_stack, &undo_top);
    redo_top = 0;
}

void undo_one(void)
{
    if (undo_top == 0) return;
    snapshots_push(redo_stack, &redo_top);
    snapshots_pop(undo_stack, &undo_top);
}

void redo_one(void)
{
    if (redo_top == 0) return;
    snapshots_push(undo_stack, &undo_top);
    snapshots_pop(redo_stack, &redo_top);
}

// --- Pattern save / load ---

#if !defined(__EMSCRIPTEN__)
static const char *patterns_dir(void)
{
    static char dir[512];
    const char *home = getenv("HOME");
    if (home)
        snprintf(dir, sizeof(dir), "%s/.wfc/patterns", home);
    else
        strncpy(dir, ".wfc/patterns", sizeof(dir));
    return dir;
}
#endif

static char *seed_string(void)
{
    size_t cap = (size_t)seed_count * 12 + 16;
    char *text = (char *)malloc(cap);
    if (!text) return NULL;
    size_t off = 0;
    text[0] = '\0';
    for (int i = 0; i < seed_count; i++)
    {
        int n = snprintf(text + off, cap - off, "%d,%d,%d;", seeds[i].x, seeds[i].y, (int)seeds[i].tile);
        if (n < 0 || (size_t)n >= cap - off) break;
        off += (size_t)n;
    }
    if (off > 0) text[off - 1] = '\0';
    return text;
}

void parse_seed_string(const char *text)
{
    if (!text || text[0] == '\0') return;
    clear_seeds();
    const char *p = text;
    while (*p && seed_count < MAX_SEEDS)
    {
        int x, y, t;
        if (sscanf(p, "%d,%d,%d", &x, &y, &t) == 3 &&
            x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE &&
            t >= 0 && t < TILE_COUNT)
        {
            place_seed(x, y, (TileType)t);
        }
        const char *semi = strchr(p, ';');
        if (!semi) break;
        p = semi + 1;
    }
}

bool save_pattern(int slot)
{
    if (slot < 1 || slot > 9) return false;
    char *text = seed_string();
    if (!text) return false;

    bool ok = false;
#if defined(__EMSCRIPTEN__)
    char key[32];
    snprintf(key, sizeof(key), "wfc_pattern_%d", slot);
    wfc_local_set(key, text);
    ok = true;
#else
    const char *dir = patterns_dir();
    char parent[512];
    const char *home = getenv("HOME");
    if (home)
        snprintf(parent, sizeof(parent), "%s/.wfc", home);
    else
        snprintf(parent, sizeof(parent), ".wfc");
    if (!DirectoryExists(parent))
        mkdir(parent, 0755);
    if (!DirectoryExists(dir))
        mkdir(dir, 0755);
    char path[640];
    snprintf(path, sizeof(path), "%s/pattern_%d.wfc", dir, slot);
    ok = SaveFileText(path, text);
#endif
    free(text);

    snprintf(notice, sizeof(notice), "Saved pattern slot %d", slot);
    notice_ticks = 120;
    return ok;
}

bool load_pattern(int slot)
{
    if (slot < 1 || slot > 9) return false;

    char *text = NULL;
#if defined(__EMSCRIPTEN__)
    char key[32];
    snprintf(key, sizeof(key), "wfc_pattern_%d", slot);
    char buf[16384];
    if (wfc_local_get(key, buf, sizeof(buf)) && buf[0] != '\0')
        text = strdup(buf);
#else
    char path[640];
    snprintf(path, sizeof(path), "%s/pattern_%d.wfc", patterns_dir(), slot);
    if (FileExists(path))
    {
        char *raw = LoadFileText(path);
        if (raw) text = raw;
    }
#endif
    if (!text)
    {
        snprintf(notice, sizeof(notice), "No saved pattern in slot %d", slot);
        notice_ticks = 120;
        return false;
    }

    undo_suppress = true;
    parse_seed_string(text);
    undo_suppress = false;
    free(text);

    snprintf(notice, sizeof(notice), "Loaded pattern slot %d", slot);
    notice_ticks = 120;
    return true;
}

// --- Export PNG ---

void export_map(void)
{
    RenderTexture2D rt = LoadRenderTexture(screenWidth, screenHeight);
    BeginTextureMode(rt);
    ClearBackground((Color){ 34, 46, 38, 255 });
    for (int x = 0; x < GRID_SIZE; x++)
        for (int y = 0; y < GRID_SIZE; y++)
        {
            Rectangle dest = { x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
            TileType t = grid[x][y].collapsed ? grid[x][y].tile : TILE_EMPTY;
            Color tint = grid[x][y].collapsed ? WHITE : (Color){ 255, 255, 255, 60 };
            DrawTexturePro(tile_textures[t],
                           (Rectangle){ 0, 0, tile_textures[t].width, tile_textures[t].height },
                           dest, (Vector2){ 0, 0 }, 0, tint);
        }
    EndTextureMode();

    Image img = LoadImageFromTexture(rt.texture);
    ImageFlipVertical(&img);
    int out_size = 0;
    unsigned char *png = ExportImageToMemory(img, ".png", &out_size);
    if (png && out_size > 0)
    {
#if defined(__EMSCRIPTEN__)
        wfc_download(png, (int)out_size, "wfc-map.png");
        snprintf(notice, sizeof(notice), "Downloaded wfc-map.png");
#else
        SaveFileData("wfc-map.png", png, out_size);
        snprintf(notice, sizeof(notice), "Saved wfc-map.png");
#endif
        notice_ticks = 150;
        free(png);
    }
    UnloadImage(img);
    UnloadRenderTexture(rt);
}

// --- URL seed handling ---

void handle_url_seeds(void)
{
#if defined(__EMSCRIPTEN__)
    char url[8192];
    if (!wfc_url_get(url, sizeof(url))) return;
    char *p = strstr(url, "seeds=");
    if (!p) return;
    p += 6;
    char *amp = strchr(p, '&');
    if (amp) *amp = '\0';
    undo_suppress = true;
    parse_seed_string(p);
    undo_suppress = false;
    if (seed_count > 0)
    {
        snprintf(notice, sizeof(notice), "Applied %d seeds from URL", seed_count);
        notice_ticks = 150;
    }
#endif
}

// --- Core grid ---

void initialize_grid(void)
{
    for (int x = 0; x < GRID_SIZE; x++)
        for (int y = 0; y < GRID_SIZE; y++)
        {
            grid[x][y].collapsed = 0;
            grid[x][y].tile = TILE_EMPTY;
            grid[x][y].possible_tiles = ((1u << TILE_COUNT) - 1);
            grid[x][y].entropy = TILE_COUNT;
        }
}

void place_seed(int x, int y, TileType tile)
{
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) return;
    grid[x][y].collapsed = 1;
    grid[x][y].tile = tile;
    grid[x][y].possible_tiles = (1u << tile);
    grid[x][y].entropy = 0;
}

void paint_seed(int x, int y, TileType tile)
{
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) return;
    if (!undo_suppress) push_undo();
    if (grid[x][y].collapsed)
    {
        for (int i = 0; i < seed_count; i++)
        {
            if (seeds[i].x == x && seeds[i].y == y)
            {
                if (seeds[i].tile == tile) return;
                place_seed(x, y, tile);
                seeds[i].tile = tile;
                return;
            }
        }
        return;
    }
    place_seed(x, y, tile);
    if (seed_count < MAX_SEEDS)
    {
        seeds[seed_count].x = x;
        seeds[seed_count].y = y;
        seeds[seed_count].tile = tile;
        seed_count++;
    }
}

void erase_seed(int x, int y)
{
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) return;
    if (!grid[x][y].collapsed) return;
    push_undo();
    grid[x][y].collapsed = 0;
    grid[x][y].tile = TILE_EMPTY;
    grid[x][y].possible_tiles = ((1u << TILE_COUNT) - 1);
    grid[x][y].entropy = TILE_COUNT;
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
    if (seed_count == 0) return;
    push_undo();
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
    for (int x = 0; x < GRID_SIZE; x++)
        for (int y = 0; y < GRID_SIZE; y++)
            if (grid[x][y].collapsed)
                propagate_cell((CellPos){ x, y });
}

bool all_collapsed(void)
{
    for (int x = 0; x < GRID_SIZE; x++)
        for (int y = 0; y < GRID_SIZE; y++)
            if (!grid[x][y].collapsed) return false;
    return true;
}

// Simple deterministic noise for tiebreaking (same seed -> same map).
static uint32_t hash_noise(int x, int y, uint32_t seed)
{
    uint32_t h = seed;
    h ^= (uint32_t)x * 0x9E3779B1u;
    h ^= (uint32_t)y * 0x85EBCA6Bu;
    h ^= h >> 13;
    h *= 0xC2B2AE35u;
    h ^= h >> 16;
    return h;
}

CellPos find_lowest_entropy_cell(void)
{
    int min_entropy = TILE_COUNT + 1;
    CellPos best = { 0, 0 };
    uint32_t best_hash = UINT32_MAX;

    for (int x = 0; x < GRID_SIZE; x++)
    {
        for (int y = 0; y < GRID_SIZE; y++)
        {
            if (grid[x][y].collapsed) continue;
            int entropy = grid[x][y].entropy;
            if (entropy > min_entropy) continue;
            uint32_t h = hash_noise(x, y, wfc_seed);
            if (entropy < min_entropy || h < best_hash)
            {
                min_entropy = entropy;
                best = (CellPos){ x, y };
                best_hash = h;
            }
        }
    }
    return best;
}

void collapse_cell(int x, int y)
{
    Cell *cell = &grid[x][y];
    TileType chosen = pick_weighted_tile(cell->possible_tiles);
    cell->collapsed = 1;
    cell->entropy = 0;
    cell->tile = chosen;
    cell->possible_tiles = (1u << chosen);
    cell_flash[x][y] = 10;
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

void propagate_cell(CellPos start)
{
    Queue q;
    if (!queue_init(&q, GRID_SIZE * GRID_SIZE * 4)) return;
    queue_enqueue(&q, start);

    while (!queue_is_empty(&q))
    {
        CellPos current;
        queue_dequeue(&q, &current);
        Cell *cell = &grid[current.x][current.y];
        if (!cell->collapsed) continue;

        for (int d = 0; d < 4; d++)
        {
            Direction dir = (Direction)d;
            CellPos neighbor_pos = get_neighbor(current, dir);

            if (neighbor_pos.x < 0 || neighbor_pos.x >= GRID_SIZE ||
                neighbor_pos.y < 0 || neighbor_pos.y >= GRID_SIZE)
                continue;

            Cell *neighbor = &grid[neighbor_pos.x][neighbor_pos.y];
            if (neighbor->collapsed) continue;

            uint32_t new_possible = 0;
            for (int t = 0; t < TILE_COUNT; t++)
            {
                if (neighbor->possible_tiles & (1u << t))
                {
                    if (can_be_adjacent(cell->tile, (TileType)t, dir))
                        new_possible |= (1u << t);
                }
            }

            if (new_possible == neighbor->possible_tiles) continue;

            neighbor->possible_tiles = new_possible;
            neighbor->entropy = tile_popcount(new_possible);
            queue_enqueue(&q, neighbor_pos);
        }
    }

    queue_free(&q);
}

void update(void)
{
    if (all_collapsed()) return;

    CellPos min_cell = find_lowest_entropy_cell();

    if (grid[min_cell.x][min_cell.y].entropy == 0)
    {
        consecutive_restarts++;
        gen_attempts++;
        if (consecutive_restarts >= MAX_RESTARTS && seed_count > 0)
        {
            snprintf(notice, sizeof(notice), "Dropped seed (%d,%d) after %d restarts",
                     seeds[0].x, seeds[0].y, gen_attempts);
            notice_ticks = 150;
            memmove(&seeds[0], &seeds[1], (size_t)(seed_count - 1) * sizeof(Seed));
            seed_count--;
            consecutive_restarts = 0;
            srand(wfc_seed ^ (uint32_t)gen_attempts);
            apply_seeds();
        }
        else
        {
            srand(wfc_seed ^ (uint32_t)gen_attempts);
            apply_seeds();
        }
        return;
    }

    collapse_cell(min_cell.x, min_cell.y);
    consecutive_restarts = 0;
    propagate_cell(min_cell);
}

void begin_generation(void)
{
    gen_attempts = 0;
    consecutive_restarts = 0;
    srand(wfc_seed);
    apply_seeds();
    state = STATE_GENERATING;
}

// --- Textures ---

bool load_tile_textures(void)
{
    for (int i = 0; i < TILE_COUNT; i++)
    {
        tile_textures[i] = LoadTexture(TILE_TEXTURE_FILES[i]);
        if (tile_textures[i].id == 0)
        {
            fprintf(stderr, "Failed to load tile texture for TileType %d (%s)\n", i, TILE_TEXTURE_FILES[i]);
            return false;
        }
    }
    return true;
}

void unload_tile_textures(void)
{
    for (int i = 0; i < TILE_COUNT; i++)
        UnloadTexture(tile_textures[i]);
}

// --- Mouse ---

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

// --- Strip / brush picker ---

#define STRIP_X 8
#define STRIP_Y 122
#define STRIP_ICON 34
#define STRIP_STEP 40

bool point_on_strip(Vector2 m)
{
    return m.x >= STRIP_X && m.x < STRIP_X + TILE_COUNT * STRIP_STEP &&
           m.y >= STRIP_Y && m.y < STRIP_Y + STRIP_ICON;
}

int strip_brush_at(Vector2 m)
{
    if (!point_on_strip(m)) return -1;
    int idx = (int)((m.x - STRIP_X) / STRIP_STEP);
    if (idx < 0 || idx >= TILE_COUNT) return -1;
    return idx;
}

// --- Input handling ---

void handle_drawing_input(int *hover_x, int *hover_y)
{
    Vector2 m = GetMousePosition();

    // Brush strip selection (click a tile to switch brush)
    int picked = strip_brush_at(m);
    if (picked >= 0 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        current_brush = picked;

    // RMB cycles tile
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        current_brush = (TileType)((current_brush + 1) % TILE_COUNT);

    bool on_strip = point_on_strip(m);

    if (*hover_x >= 0 && *hover_y >= 0 && !on_strip)
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            paint_seed(*hover_x, *hover_y, (TileType)current_brush);
        if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON) || IsKeyDown(KEY_E))
            erase_seed(*hover_x, *hover_y);
    }

    if (IsKeyPressed(KEY_R)) clear_seeds();
    if (IsKeyPressed(KEY_LEFT_BRACKET))
        wfc_seed = (wfc_seed + 999999u) % 1000000u;
    if (IsKeyPressed(KEY_RIGHT_BRACKET))
        wfc_seed = (wfc_seed + 1u) % 1000000u;

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
        begin_generation();

    if (IsKeyPressed(KEY_P)) export_map();

    if (IsKeyPressed(KEY_Z) && IsKeyDown(KEY_LEFT_CONTROL))
    {
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            redo_one();
        else
            undo_one();
    }

    bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    for (int s = 1; s <= 9; s++)
    {
        if (!IsKeyPressed(KEY_ZERO + s)) continue;
        if (ctrl) save_pattern(s);
        else load_pattern(s);
    }
}

void handle_generating_input(void)
{
    if (IsKeyPressed(KEY_R)) begin_generation();

    if (IsKeyPressed(KEY_M))
    {
        redraw_seeds();
        state = STATE_DRAWING;
    }

    if (IsKeyPressed(KEY_SPACE)) update();

    if (IsKeyPressed(KEY_LEFT_BRACKET) && cells_per_frame > 1) cells_per_frame--;
    if (IsKeyPressed(KEY_RIGHT_BRACKET) && cells_per_frame < 200) cells_per_frame++;

    if (IsKeyPressed(KEY_P)) export_map();

    if (!all_collapsed())
    {
        for (int i = 0; i < cells_per_frame; i++)
            update();
    }
}

// --- Drawing ---

void draw_grid(int hover_x, int hover_y)
{
    ClearBackground((Color){ 40, 52, 44, 255 });
    collapsed_count = 0;

    for (int x = 0; x < GRID_SIZE; x++)
        for (int y = 0; y < GRID_SIZE; y++)
        {
            Rectangle dest = { x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
            TileType t = grid[x][y].collapsed ? grid[x][y].tile : TILE_EMPTY;

            if (grid[x][y].collapsed)
            {
                collapsed_count++;
                DrawTexturePro(tile_textures[t],
                               (Rectangle){ 0, 0, tile_textures[t].width, tile_textures[t].height },
                               dest, (Vector2){ 0, 0 }, 0, WHITE);
                if (cell_flash[x][y] > 0)
                {
                    DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE,
                                  (Color){ 255, 255, 255, cell_flash[x][y] * 25 });
                    cell_flash[x][y]--;
                }
            }
            else
            {
                int alpha = 45 + (int)((1.0f - grid[x][y].entropy / (float)TILE_COUNT) * 200.0f);
                DrawTexturePro(tile_textures[TILE_EMPTY],
                               (Rectangle){ 0, 0, tile_textures[TILE_EMPTY].width, tile_textures[TILE_EMPTY].height },
                               dest, (Vector2){ 0, 0 }, 0, (Color){ 255, 255, 255, (unsigned char)alpha });
            }
        }

    // Hover ghost
    if (state == STATE_DRAWING && hover_x >= 0 && hover_y >= 0)
    {
        int icon_x = hover_x * CELL_SIZE;
        int icon_y = hover_y * CELL_SIZE;
        Color ghost_color = (Color){ 255, 255, 255, 120 };
        DrawRectangle(icon_x, icon_y, CELL_SIZE, CELL_SIZE, ghost_color);
        DrawTexturePro(tile_textures[current_brush],
                       (Rectangle){ 0, 0, tile_textures[current_brush].width, tile_textures[current_brush].height },
                       (Rectangle){ icon_x, icon_y, CELL_SIZE, CELL_SIZE },
                       (Vector2){ 0, 0 }, 0, WHITE);
    }
}

void draw_hud(int hover_x, int hover_y)
{
    // Hover cell highlight
    if (hover_x >= 0 && hover_y >= 0)
        DrawRectangleLines(hover_x * CELL_SIZE, hover_y * CELL_SIZE, CELL_SIZE, CELL_SIZE, RED);

    // Brush panel (top-left)
    const int panel_w = 236;
    const int panel_h = 62;
    DrawRectangle(8, 8, panel_w, panel_h, (Color){ 0, 0, 0, 160 });

    Rectangle icon = { 16, 16, 40, 40 };
    DrawTexturePro(tile_textures[current_brush],
                   (Rectangle){ 0, 0, tile_textures[current_brush].width, tile_textures[current_brush].height },
                   icon, (Vector2){ 0, 0 }, 0, WHITE);
    DrawRectangleLines((int)icon.x, (int)icon.y, (int)icon.width, (int)icon.height, RED);

    char brush_line[96];
    snprintf(brush_line, sizeof(brush_line), "Brush: %s", TILE_NAMES[current_brush]);
    DrawText(brush_line, 66, 12, 14, WHITE);

    char seed_line[96];
    snprintf(seed_line, sizeof(seed_line), "Seeds: %d   Seed: %u", seed_count, wfc_seed);
    DrawText(seed_line, 66, 32, 14, WHITE);

    // Brush strip: 12 clickable tiles
    Vector2 m = GetMousePosition();
    for (int i = 0; i < TILE_COUNT; i++)
    {
        int ix = STRIP_X + i * STRIP_STEP;
        int iy = STRIP_Y;
        Rectangle r = { ix, iy, STRIP_ICON, STRIP_ICON };
        bool hovering = CheckCollisionPointRec(m, r);
        bool selected = (i == current_brush);
        Color fill = hovering ? (Color){ 70, 110, 70, 255 }
                              : (selected ? (Color){ 90, 60, 20, 255 } : (Color){ 24, 34, 24, 255 });
        DrawRectangleRec(r, fill);
        DrawTexturePro(tile_textures[i],
                       (Rectangle){ 0, 0, tile_textures[i].width, tile_textures[i].height },
                       r, (Vector2){ 0, 0 }, 0, WHITE);
        DrawRectangleLines(ix, iy, STRIP_ICON, STRIP_ICON, selected ? GOLD : DARKGRAY);
    }

    // Bottom hint bar
    DrawRectangle(8, screenHeight - 34, screenWidth - 16, 26, (Color){ 0, 0, 0, 160 });
    DrawText("LMB paint | RMB cycle | MMB/E erase | R clear | Ctrl+Z/Shift+Z undo/redo | 1-9 load / Ctrl+1-9 save | P PNG | [ ] seed | Enter generate",
             16, screenHeight - 30, 12, WHITE);

    // Notice text (above the hint bar)
    if (notice_ticks > 0)
    {
        DrawText(notice, 16, screenHeight - 48, 14, LIME);
        notice_ticks--;
    }

    // Stats HUD (top-right)
    char stats[160];
    snprintf(stats, sizeof(stats), "Collapsed: %d/%d   Attempts: %d   Restarts: %d   Cells/s: %d   FPS: %d",
             collapsed_count, GRID_SIZE * GRID_SIZE, gen_attempts,
             consecutive_restarts, cells_per_frame, GetFPS());
    DrawText(stats, screenWidth - MeasureText(stats, 16) - 16, 10, 16, WHITE);
}

// --- Main ---

int main(void)
{
    InitWindow(screenWidth, screenHeight, "WFC road map generator");
    SetTargetFPS(60);

    if (!load_tile_textures())
    {
        CloseWindow();
        return 1;
    }

    handle_url_seeds();
    state = STATE_DRAWING;

    while (!WindowShouldClose())
    {
        int hx = -1, hy = -1;
        mouse_to_cell(&hx, &hy);

        if (state == STATE_DRAWING)
            handle_drawing_input(&hx, &hy);
        else
            handle_generating_input();

        BeginDrawing();
        draw_grid(hx, hy);
        if (state == STATE_DRAWING)
            draw_hud(hx, hy);
        EndDrawing();
    }

    unload_tile_textures();
    CloseWindow();
    return 0;
}
