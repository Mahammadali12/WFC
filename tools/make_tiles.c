// Tool: generates the 5 missing road tiles (cross + 4 T-junctions) in the
// same visual style as the existing tilesets/*.png.
// Usage: bin/make_tiles   (run from repo root, like the game)
// Output: tilesets/crossing.png, tilesets/t-{up,down,left,right}.png
#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>

#define TS 256

typedef struct {
    Color road;
    int y0, y1;         // horizontal band row range
    int x0, x1;         // vertical band column range
} Palette;

static bool pixel_is_road(Color a, Color b)
{
    int diff = abs(a.r - b.r) + abs(a.g - b.g) + abs(a.b - b.b);
    return diff > 60;
}

static bool sample(Image grass, const char *path, Palette *out)
{
    Image img = LoadImage(path);
    if (img.data == NULL)
    {
        fprintf(stderr, "Cannot load %s (run from repo root)\n", path);
        return false;
    }

    long r = 0, g = 0, b = 0;
    int road_pixels = 0;
    int row_count[TS] = {0};
    int col_count[TS] = {0};

    for (int y = 0; y < TS; y++)
    {
        for (int x = 0; x < TS; x++)
        {
            Color c = GetImageColor(img, x, y);
            Color base = GetImageColor(grass, x, y);
            if (pixel_is_road(c, base))
            {
                r += c.r; g += c.g; b += c.b;
                road_pixels++;
                row_count[y]++;
                col_count[x]++;
            }
        }
    }
    UnloadImage(img);

    if (road_pixels < 100)
    {
        fprintf(stderr, "%s: too few road pixels (%d)\n", path, road_pixels);
        return false;
    }

    out->road = (Color){ (unsigned char)(r / road_pixels),
                         (unsigned char)(g / road_pixels),
                         (unsigned char)(b / road_pixels), 255 };

    int row_lo = TS, row_hi = -1, col_lo = TS, col_hi = -1;
    for (int i = 0; i < TS; i++)
    {
        if (row_count[i] > TS * 60 / 100) { if (i < row_lo) row_lo = i; row_hi = i; }
        if (col_count[i] > TS * 60 / 100) { if (i < col_lo) col_lo = i; col_hi = i; }
    }

    if (row_hi < row_lo || col_hi < col_lo)
    {
        fprintf(stderr, "%s: no dense band found\n", path);
        return false;
    }

    out->y0 = row_lo; out->y1 = row_hi;
    out->x0 = col_lo; out->x1 = col_hi;

    printf("%-30s road=#%02X%02X%02X H[y=%d..%d] V[x=%d..%d] px=%d\n",
           path, out->road.r, out->road.g, out->road.b, row_lo, row_hi, col_lo, col_hi, road_pixels);
    return true;
}

static void hband(Image *img, Palette p)
{
    ImageDrawRectangle(img, 0, p.y0, TS, p.y1 - p.y0 + 1, p.road);
}

static void vband(Image *img, Palette p)
{
    ImageDrawRectangle(img, p.x0, 0, p.x1 - p.x0 + 1, TS, p.road);
}

// Vertical road segment, restricted to the row range [y0, y1]
static Image vpart(Image base, Palette p, int y0, int y1)
{
    ImageDrawRectangle(&base, p.x0, y0, p.x1 - p.x0 + 1, y1 - y0 + 1, p.road);
    return base;
}

// Horizontal road segment, restricted to the column range [x0, x1]
static Image hpart(Image base, Palette p, int x0, int x1)
{
    ImageDrawRectangle(&base, x0, p.y0, x1 - x0 + 1, p.y1 - p.y0 + 1, p.road);
    return base;
}

static void export_tile(Image img, const char *path)
{
    if (!ExportImage(img, path))
        fprintf(stderr, "ExportImage failed for %s\n", path);
    UnloadImage(img);
}

int main(void)
{
    Image grass = LoadImage("tilesets/empty-green.png");
    if (grass.data == NULL)
    {
        fprintf(stderr, "tilesets/empty-green.png not found (run from repo root)\n");
        return 1;
    }

    Palette h, v;
    if (!sample(grass, "tilesets/horizontal-line.png", &h))
    {
        UnloadImage(grass);
        return 1;
    }
    if (!sample(grass, "tilesets/vertical-line.png", &v))
    {
        UnloadImage(grass);
        return 1;
    }

    Palette p = h;                 // road color + horizontal band rows from HR
    p.x0 = v.x0; p.x1 = v.x1;      // vertical band columns from VR
    int hym = (p.y0 + p.y1) / 2;   // horizontal band middle row
    int hxm = (p.x0 + p.x1) / 2;   // vertical band middle column

    Image cross = ImageCopy(grass);
    hband(&cross, p);
    vband(&cross, p);
    export_tile(cross, "tilesets/crossing.png");

    Image tup = ImageCopy(grass);
    hband(&tup, p);
    tup = vpart(tup, p, 0, hym);
    export_tile(tup, "tilesets/t-up.png");

    Image tdown = ImageCopy(grass);
    hband(&tdown, p);
    tdown = vpart(tdown, p, hym, TS - 1);
    export_tile(tdown, "tilesets/t-down.png");

    Image tleft = ImageCopy(grass);
    vband(&tleft, p);
    tleft = hpart(tleft, p, 0, hxm);
    export_tile(tleft, "tilesets/t-left.png");

    Image tright = ImageCopy(grass);
    vband(&tright, p);
    tright = hpart(tright, p, hxm, TS - 1);
    export_tile(tright, "tilesets/t-right.png");

    UnloadImage(grass);
    printf("done: tilesets/crossing.png tilesets/t-{up,down,left,right}.png\n");
    return 0;
}