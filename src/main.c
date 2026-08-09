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

EM_JS(void, js_get_window_size, (int *w, int *h), {
    setValue(w, window.innerWidth, 'i32');
    setValue(h, window.innerHeight, 'i32');
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

EM_JS(void, js_hud_update, (
    const char* brush,
    int seeds,
    unsigned int seed,
    int collapsed,
    int total,
    int attempts,
    int restarts,
    int fps,
    int speed,
    int heatmap,
    const char* notice,
    int notice_ticks
), {
    var elBrush = document.getElementById('hud-brush');
    if (!elBrush) return;

    elBrush.textContent = UTF8ToString(brush);
    document.getElementById('hud-seeds').textContent = seeds;
    document.getElementById('hud-wfc-seed').textContent = seed;
    document.getElementById('hud-collapsed').textContent = collapsed;
    document.getElementById('hud-total').textContent = total;
    document.getElementById('hud-attempts').textContent = attempts;
    document.getElementById('hud-restarts').textContent = restarts;
    document.getElementById('hud-fps').textContent = fps;
    document.getElementById('hud-speed').textContent = speed;
    document.getElementById('hud-heatmap-badge').style.display = heatmap ? 'inline-block' : 'none';

    var el = document.getElementById('hud-notice');
    if (notice_ticks > 0) {
        el.textContent = UTF8ToString(notice);
        el.classList.add('show');
    } else {
        el.classList.remove('show');
    }
});
#endif

// --- Tile texture path prefix ---
// Desktop loads from assets/tilesets/; web preload mounts assets/tilesets at /tilesets.
#if defined(__EMSCRIPTEN__)
#define TS_PREFIX "tilesets/"
#else
#define TS_PREFIX "assets/tilesets/"
#endif

// --- Tile texture file table ---
const char *TILE_TEXTURE_FILES[TILE_COUNT] = {
    [TILE_HR]      = TS_PREFIX "horizontal-line.png",
    [TILE_VR]      = TS_PREFIX "vertical-line.png",
    [TILE_UP_L]    = TS_PREFIX "upper-left-corner.png",
    [TILE_UP_R]    = TS_PREFIX "upper-right-corner.png",
    [TILE_LOW_L]   = TS_PREFIX "lower-left-corner.png",
    [TILE_LOW_R]   = TS_PREFIX "lower-right-corner.png",
    [TILE_EMPTY]   = TS_PREFIX "empty-green.png",

    [TILE_CROSS]   = TS_PREFIX "crossing.png",
    [TILE_T_UP]    = TS_PREFIX "t-up.png",
    [TILE_T_DOWN]  = TS_PREFIX "t-down.png",
    [TILE_T_LEFT]  = TS_PREFIX "t-left.png",
    [TILE_T_RIGHT] = TS_PREFIX "t-right.png",

    [TILE_WATER_HR]      = TS_PREFIX "water-hr.png",
    [TILE_WATER_VR]      = TS_PREFIX "water-vr.png",
    [TILE_WATER_UP_L]    = TS_PREFIX "water-up-l.png",
    [TILE_WATER_UP_R]    = TS_PREFIX "water-up-r.png",
    [TILE_WATER_LOW_L]   = TS_PREFIX "water-low-l.png",
    [TILE_WATER_LOW_R]   = TS_PREFIX "water-low-r.png",

    [TILE_HR_BANK_TOP]    = TS_PREFIX "horizontal-line.png",
    [TILE_HR_BANK_BOTTOM] = TS_PREFIX "horizontal-line.png",
    [TILE_HR_BANK_BOTH]   = TS_PREFIX "horizontal-line.png",
    [TILE_VR_BANK_LEFT]   = TS_PREFIX "vertical-line.png",
    [TILE_VR_BANK_RIGHT]  = TS_PREFIX "vertical-line.png",
    [TILE_VR_BANK_BOTH]   = TS_PREFIX "vertical-line.png",

    [TILE_WATER_HR_BANK_TOP]    = TS_PREFIX "water-hr.png",
    [TILE_WATER_HR_BANK_BOTTOM] = TS_PREFIX "water-hr.png",
    [TILE_WATER_HR_BANK_BOTH]   = TS_PREFIX "water-hr.png",
    [TILE_WATER_VR_BANK_LEFT]   = TS_PREFIX "water-vr.png",
    [TILE_WATER_VR_BANK_RIGHT]  = TS_PREFIX "water-vr.png",
    [TILE_WATER_VR_BANK_BOTH]   = TS_PREFIX "water-vr.png",

    [TILE_UP_L_BANK_TOP]      = TS_PREFIX "upper-left-corner.png",
    [TILE_UP_L_BANK_LEFT]     = TS_PREFIX "upper-left-corner.png",
    [TILE_UP_L_BANK_BOTH]     = TS_PREFIX "upper-left-corner.png",
    [TILE_UP_R_BANK_TOP]      = TS_PREFIX "upper-right-corner.png",
    [TILE_UP_R_BANK_RIGHT]    = TS_PREFIX "upper-right-corner.png",
    [TILE_UP_R_BANK_BOTH]     = TS_PREFIX "upper-right-corner.png",
    [TILE_LOW_L_BANK_BOTTOM]  = TS_PREFIX "lower-left-corner.png",
    [TILE_LOW_L_BANK_LEFT]    = TS_PREFIX "lower-left-corner.png",
    [TILE_LOW_L_BANK_BOTH]    = TS_PREFIX "lower-left-corner.png",
    [TILE_LOW_R_BANK_BOTTOM]  = TS_PREFIX "lower-right-corner.png",
    [TILE_LOW_R_BANK_RIGHT]   = TS_PREFIX "lower-right-corner.png",
    [TILE_LOW_R_BANK_BOTH]    = TS_PREFIX "lower-right-corner.png",

    [TILE_WATER_UP_L_BANK_TOP]      = TS_PREFIX "water-up-l.png",
    [TILE_WATER_UP_L_BANK_LEFT]     = TS_PREFIX "water-up-l.png",
    [TILE_WATER_UP_L_BANK_BOTH]     = TS_PREFIX "water-up-l.png",
    [TILE_WATER_UP_R_BANK_TOP]      = TS_PREFIX "water-up-r.png",
    [TILE_WATER_UP_R_BANK_RIGHT]    = TS_PREFIX "water-up-r.png",
    [TILE_WATER_UP_R_BANK_BOTH]     = TS_PREFIX "water-up-r.png",
    [TILE_WATER_LOW_L_BANK_BOTTOM]  = TS_PREFIX "water-low-l.png",
    [TILE_WATER_LOW_L_BANK_LEFT]    = TS_PREFIX "water-low-l.png",
    [TILE_WATER_LOW_L_BANK_BOTH]    = TS_PREFIX "water-low-l.png",
    [TILE_WATER_LOW_R_BANK_BOTTOM]  = TS_PREFIX "water-low-r.png",
    [TILE_WATER_LOW_R_BANK_RIGHT]   = TS_PREFIX "water-low-r.png",
    [TILE_WATER_LOW_R_BANK_BOTH]    = TS_PREFIX "water-low-r.png",

    [TILE_GRASS_BANK_TOP]    = TS_PREFIX "empty-green.png",
    [TILE_GRASS_BANK_BOTTOM] = TS_PREFIX "empty-green.png",
    [TILE_GRASS_BANK_LEFT]   = TS_PREFIX "empty-green.png",
    [TILE_GRASS_BANK_RIGHT]  = TS_PREFIX "empty-green.png",
};

// --- Structs ---
typedef struct {
    int collapsed;
    TileType tile;
    uint64_t possible_tiles;
    int entropy;
} Cell;

typedef struct {
    int x;
    int y;
    TileType tile;
} Seed;

typedef struct {
    Seed *data;
    int len;
} SeedSnapshot;

// --- Dynamic grid config ---
int GRID_W = 50;
int GRID_H = 50;
int CELL_SIZE = 28;
int max_seeds = 2500;

// --- Dynamic arrays ---
Cell *grid = NULL;
uint8_t *cell_flash = NULL;
Seed *seeds = NULL;

#define CELL(x, y)     (grid[(y) * GRID_W + (x)])
#define FLASH(x, y)    (cell_flash[(y) * GRID_W + (x)])

int seed_count = 0;
int current_brush = TILE_HR;

int screenWidth = 1400;
int screenHeight = 1400;

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
bool show_heatmap = false;

char notice[160];
int notice_ticks = 0;

#define UNDO_DEPTH 64
#define MAX_RESTARTS 50

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
void copy_seeds_to_clipboard(void);
void paste_seeds_from_clipboard(void);
void undo_one(void);
void redo_one(void);

static void update_web_hud(void)
{
#if defined(__EMSCRIPTEN__)
    js_hud_update(
        TILE_NAMES[current_brush],
        seed_count,
        wfc_seed,
        collapsed_count,
        GRID_W * GRID_H,
        gen_attempts,
        consecutive_restarts,
        GetFPS(),
        cells_per_frame,
        show_heatmap,
        notice,
        notice_ticks
    );
#endif
}

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
    while (*p && seed_count < max_seeds)
    {
        int x, y, t;
        if (sscanf(p, "%d,%d,%d", &x, &y, &t) == 3 &&
            x >= 0 && x < GRID_W && y >= 0 && y < GRID_H &&
            t >= 0 && t < TILE_COUNT)
        {
            place_seed(x, y, (TileType)t);
        }
        const char *semi = strchr(p, ';');
        if (!semi) break;
        p = semi + 1;
    }
}

void copy_seeds_to_clipboard(void)
{
    char *text = seed_string();
    if (!text)
    {
        snprintf(notice, sizeof(notice), "Copy failed (out of memory)");
        notice_ticks = 120;
        return;
    }
    SetClipboardText(text);
    free(text);

    snprintf(notice, sizeof(notice), "Copied %d seeds to clipboard", seed_count);
    notice_ticks = 120;
}

void paste_seeds_from_clipboard(void)
{
    const char *text = GetClipboardText();
    if (!text || text[0] == '\0')
    {
        snprintf(notice, sizeof(notice), "Clipboard empty");
        notice_ticks = 120;
        return;
    }
    if (!strchr(text, ',') || !strchr(text, ';'))
    {
        snprintf(notice, sizeof(notice), "Clipboard not a seed pattern");
        notice_ticks = 120;
        return;
    }
    int prev = seed_count;
    undo_suppress = true;
    parse_seed_string(text);
    undo_suppress = false;
    if (seed_count != prev)
        push_undo();
    snprintf(notice, sizeof(notice), "Pasted %d seeds from clipboard", seed_count);
    notice_ticks = 120;
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
    for (int x = 0; x < GRID_W; x++)
        for (int y = 0; y < GRID_H; y++)
        {
            Rectangle dest = { x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
            TileType t = CELL(x, y).collapsed ? CELL(x, y).tile : TILE_EMPTY;
            Color tint = CELL(x, y).collapsed ? WHITE : (Color){ 255, 255, 255, 60 };
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
    for (int x = 0; x < GRID_W; x++)
        for (int y = 0; y < GRID_H; y++)
        {
            CELL(x, y).collapsed = 0;
            CELL(x, y).tile = TILE_EMPTY;
            CELL(x, y).possible_tiles = ((1ULL << TILE_COUNT) - 1);
            CELL(x, y).entropy = TILE_COUNT;
        }
}

void place_seed(int x, int y, TileType tile)
{
    if (x < 0 || x >= GRID_W || y < 0 || y >= GRID_H) return;
    CELL(x, y).collapsed = 1;
    CELL(x, y).tile = tile;
    CELL(x, y).possible_tiles = (1ULL << tile);
    CELL(x, y).entropy = 0;
}

void paint_seed(int x, int y, TileType tile)
{
    if (x < 0 || x >= GRID_W || y < 0 || y >= GRID_H) return;
    if (!undo_suppress) push_undo();
    if (CELL(x, y).collapsed)
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
    if (seed_count < max_seeds)
    {
        seeds[seed_count].x = x;
        seeds[seed_count].y = y;
        seeds[seed_count].tile = tile;
        seed_count++;
    }
}

void erase_seed(int x, int y)
{
    if (x < 0 || x >= GRID_W || y < 0 || y >= GRID_H) return;
    if (!CELL(x, y).collapsed) return;
    push_undo();
    CELL(x, y).collapsed = 0;
    CELL(x, y).tile = TILE_EMPTY;
    CELL(x, y).possible_tiles = ((1ULL << TILE_COUNT) - 1);
    CELL(x, y).entropy = TILE_COUNT;
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
    for (int x = 0; x < GRID_W; x++)
        for (int y = 0; y < GRID_H; y++)
            if (CELL(x, y).collapsed)
                propagate_cell((CellPos){ x, y });
}

bool all_collapsed(void)
{
    for (int x = 0; x < GRID_W; x++)
        for (int y = 0; y < GRID_H; y++)
            if (!CELL(x, y).collapsed) return false;
    return true;
}

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

    for (int x = 0; x < GRID_W; x++)
    {
        for (int y = 0; y < GRID_H; y++)
        {
            if (CELL(x, y).collapsed) continue;
            int entropy = CELL(x, y).entropy;
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
    Cell *cell = &CELL(x, y);
    TileType chosen = pick_weighted_tile(cell->possible_tiles);
    cell->collapsed = 1;
    cell->entropy = 0;
    cell->tile = chosen;
    cell->possible_tiles = (1ULL << chosen);
    FLASH(x, y) = 10;
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
    if (!queue_init(&q, GRID_W * GRID_H * 16)) return;
    queue_enqueue(&q, start);

    while (!queue_is_empty(&q))
    {
        CellPos current;
        queue_dequeue(&q, &current);

        Cell *cell = &CELL(current.x, current.y);
        if (cell->entropy == 0 && !cell->collapsed)
            continue;

        for (int d = 0; d < 4; d++)
        {
            Direction dir = (Direction)d;
            CellPos neighbor_pos = get_neighbor(current, dir);

            if (neighbor_pos.x < 0 || neighbor_pos.x >= GRID_W ||
                neighbor_pos.y < 0 || neighbor_pos.y >= GRID_H)
                continue;

            Cell *neighbor = &CELL(neighbor_pos.x, neighbor_pos.y);
            if (neighbor->collapsed)
                continue;

            uint64_t new_possible = 0;
            for (int t = 0; t < TILE_COUNT; t++)
            {
                if (!(neighbor->possible_tiles & (1ULL << t)))
                    continue;

                bool ok = false;
                for (int s = 0; s < TILE_COUNT; s++)
                {
                    if (cell->possible_tiles & (1ULL << s))
                    {
                        if (can_be_adjacent((TileType)s, (TileType)t, dir))
                        {
                            ok = true;
                            break;
                        }
                    }
                }
                if (ok)
                    new_possible |= (1ULL << t);
            }

            if (new_possible == neighbor->possible_tiles)
                continue;

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

    if (CELL(min_cell.x, min_cell.y).entropy == 0)
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
    if (x < 0 || x >= GRID_W || y < 0 || y >= GRID_H)
        return false;
    *out_x = x;
    *out_y = y;
    return true;
}

// --- Strip / brush picker ---
#define STRIP_X 8
#define STRIP_Y 8
#define STRIP_ICON 34
#define STRIP_STEP 40
#define STRIP_ICONS_PER_ROW 30

bool point_on_strip(Vector2 m)
{
    int rows = (TILE_COUNT + STRIP_ICONS_PER_ROW - 1) / STRIP_ICONS_PER_ROW;
    float width = STRIP_ICONS_PER_ROW * STRIP_STEP;
    float height = rows * STRIP_STEP;
    return m.x >= STRIP_X && m.x < STRIP_X + width &&
           m.y >= STRIP_Y && m.y < STRIP_Y + height;
}

int strip_brush_at(Vector2 m)
{
    if (!point_on_strip(m)) return -1;
    int col = (int)((m.x - STRIP_X) / STRIP_STEP);
    int row = (int)((m.y - STRIP_Y) / STRIP_STEP);
    int idx = row * STRIP_ICONS_PER_ROW + col;
    if (idx < 0 || idx >= TILE_COUNT) return -1;
    return idx;
}

void draw_brush_strip(void)
{
    Vector2 m = GetMousePosition();
    for (int i = 0; i < TILE_COUNT; i++)
    {
        int row = i / STRIP_ICONS_PER_ROW;
        int col = i % STRIP_ICONS_PER_ROW;
        int ix = STRIP_X + col * STRIP_STEP;
        int iy = STRIP_Y + row * STRIP_STEP;
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
}

// --- Input handling ---

void handle_drawing_input(int *hover_x, int *hover_y)
{
    Vector2 m = GetMousePosition();

    int picked = strip_brush_at(m);
    if (picked >= 0 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        current_brush = picked;

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

    if (IsKeyPressed(KEY_H))
    {
        show_heatmap = !show_heatmap;
        snprintf(notice, sizeof(notice), "Heatmap: %s", show_heatmap ? "ON" : "OFF");
        notice_ticks = 90;
    }

    if (IsKeyPressed(KEY_C) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL))
        copy_seeds_to_clipboard();

    if (IsKeyPressed(KEY_V) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL))
        paste_seeds_from_clipboard();

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

    if (IsKeyPressed(KEY_H))
    {
        show_heatmap = !show_heatmap;
        snprintf(notice, sizeof(notice), "Heatmap: %s", show_heatmap ? "ON" : "OFF");
        notice_ticks = 90;
    }

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

    for (int x = 0; x < GRID_W; x++)
        for (int y = 0; y < GRID_H; y++)
        {
            Rectangle dest = { x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
            TileType t = CELL(x, y).collapsed ? CELL(x, y).tile : TILE_EMPTY;

            if (CELL(x, y).collapsed)
            {
                collapsed_count++;
                DrawTexturePro(tile_textures[t],
                               (Rectangle){ 0, 0, tile_textures[t].width, tile_textures[t].height },
                               dest, (Vector2){ 0, 0 }, 0, WHITE);
                if (FLASH(x, y) > 0)
                {
                    DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE,
                                  (Color){ 255, 255, 255, FLASH(x, y) * 25 });
                    FLASH(x, y)--;
                }
            }
            else if (show_heatmap)
            {
                float frac = CELL(x, y).entropy / (float)TILE_COUNT;
                if (frac < 0.0f) frac = 0.0f;
                if (frac > 1.0f) frac = 1.0f;
                unsigned char r = (unsigned char)(60 + 195.0f * frac);
                unsigned char b = (unsigned char)(60 + 195.0f * (1.0f - frac));
                DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE,
                              (Color){ r, 80, b, 220 });
                char num[4];
                snprintf(num, sizeof(num), "%d", CELL(x, y).entropy);
                int text_w = MeasureText(num, 10);
                DrawText(num,
                         x * CELL_SIZE + (CELL_SIZE - text_w) / 2,
                         y * CELL_SIZE + (CELL_SIZE - 10) / 2,
                         10, BLACK);
            }
            else
            {
                int alpha = 45 + (int)((1.0f - CELL(x, y).entropy / (float)TILE_COUNT) * 200.0f);
                DrawTexturePro(tile_textures[TILE_EMPTY],
                               (Rectangle){ 0, 0, tile_textures[TILE_EMPTY].width, tile_textures[TILE_EMPTY].height },
                               dest, (Vector2){ 0, 0 }, 0, (Color){ 255, 255, 255, (unsigned char)alpha });
            }
        }

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
    if (hover_x >= 0 && hover_y >= 0)
        DrawRectangleLines(hover_x * CELL_SIZE, hover_y * CELL_SIZE, CELL_SIZE, CELL_SIZE, RED);

    draw_brush_strip();

#if !defined(__EMSCRIPTEN__)
    DrawRectangle(8, 8, 236, 62, (Color){ 0, 0, 0, 160 });
    DrawText(TextFormat("Brush: %s", TILE_NAMES[current_brush]), 16, 14, 18, WHITE);
    DrawText(TextFormat("Seeds: %d  Seed: %06u", seed_count, wfc_seed), 16, 38, 16, LIGHTGRAY);
#endif
}

// --- Main ---

int main(void)
{
#if defined(__EMSCRIPTEN__)
    int winW = 1280, winH = 720;
    js_get_window_size(&winW, &winH);
    CELL_SIZE = 28;
    GRID_W = winW / CELL_SIZE;
    GRID_H = winH / CELL_SIZE;
    if (GRID_W < 10) GRID_W = 10;
    if (GRID_H < 10) GRID_H = 10;
#else
    GRID_W = 50;
    GRID_H = 50;
    CELL_SIZE = 28;
#endif

    screenWidth = GRID_W * CELL_SIZE;
    screenHeight = GRID_H * CELL_SIZE;
    max_seeds = GRID_W * GRID_H;

    grid = (Cell *)calloc((size_t)GRID_W * GRID_H, sizeof(Cell));
    cell_flash = (uint8_t *)calloc((size_t)GRID_W * GRID_H, sizeof(uint8_t));
    seeds = (Seed *)malloc((size_t)max_seeds * sizeof(Seed));
    if (!grid || !cell_flash || !seeds)
    {
        fprintf(stderr, "Failed to allocate grid memory\n");
        return 1;
    }

    InitWindow(screenWidth, screenHeight, "WFC Road Map");
    SetTargetFPS(60);

    if (!load_tile_textures())
    {
        CloseWindow();
        free(grid);
        free(cell_flash);
        free(seeds);
        return 1;
    }

    initialize_grid();
    handle_url_seeds();
    state = STATE_DRAWING;

    while (!WindowShouldClose())
    {
        int hx = -1, hy = -1;
        mouse_to_cell(&hx, &hy);

        if (state == STATE_DRAWING)
        {
            handle_drawing_input(&hx, &hy);
        }
        else
        {
            handle_generating_input();
        }

        update_web_hud();

        BeginDrawing();
        draw_grid(hx, hy);
        if (state == STATE_DRAWING)
            draw_hud(hx, hy);
        EndDrawing();
    }

    unload_tile_textures();
    CloseWindow();

    for (int i = 0; i < undo_top; i++) free(undo_stack[i].data);
    for (int i = 0; i < redo_top; i++) free(redo_stack[i].data);
    free(grid);
    free(cell_flash);
    free(seeds);

    return 0;
}
