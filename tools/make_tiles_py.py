#!/usr/bin/env python3
"""
Procedural tile art generator for the WFC road/water/bank system.

Generates 256x256 PNGs for the 46 new tiles (water, bank variants, grass
banks) using the same visual language as the existing tileset (the road
tiles use a 60-pixel-wide gray band on green; we mirror that).

This is the Python counterpart of tools/make_tiles.c (which only handled
crossing + 4 T-junctions). We use Python+PIL here because the new tile
set has enough variations that hand-coding each one in C is more error-
prone than a small data-driven generator.

Color palette (matched to the existing tileset):
  - grass:    (74, 122, 80)    #4a7a50  - the green of empty-green.png
  - road:     (88, 88, 88)     #585858  - the gray of horizontal-line.png
  - water:    (62, 102, 140)   #3e668c  - blue, slightly desaturated
  - bank:     (180, 158, 110)  #b49e6e  - sandy/tan embankment

Bank is rendered as a sand-colored band, narrower than a road, on the
side(s) of the cell that have a BANK edge. The cell still shows the
"main" feature (road or water) on the road/water side(s).
"""

import os
import sys
from PIL import Image, ImageDraw

# ---- Colors ----------------------------------------------------------------
GRASS = (74, 122, 80, 255)
ROAD  = (88, 88, 88, 255)
WATER = (62, 102, 140, 255)
BANK  = (180, 158, 110, 255)

# Geometry matched to the existing tiles (256x256, 60px band):
SIZE = 256
BAND = 60  # width of road/water band
BANK_BAND = 22  # width of the bank strip (narrower than a road)
HALF = SIZE // 2  # 128

OUT_DIR = os.path.join(os.path.dirname(__file__), "..", "tilesets")


def fill_solid(img, color):
    """Fill the entire image with a single color."""
    draw = ImageDraw.Draw(img)
    draw.rectangle([(0, 0), (SIZE, SIZE)], fill=color)


def fill_grass(img):
    fill_solid(img, GRASS)


def draw_road_h(img, top, bottom):
    """Horizontal road: a vertical band from top to bottom of the cell.
    top/bottom are 'cropped' bounds — None means full extent."""
    draw = ImageDraw.Draw(img)
    x0 = HALF - BAND // 2
    x1 = HALF + BAND // 2
    draw.rectangle([(x0, 0 if top is None else top),
                    (x1, SIZE if bottom is None else bottom)], fill=ROAD)


def draw_road_v(img, left, right):
    draw = ImageDraw.Draw(img)
    y0 = HALF - BAND // 2
    y1 = HALF + BAND // 2
    draw.rectangle([(0 if left is None else left, y0),
                    (SIZE if right is None else right, y1)], fill=ROAD)


def draw_water_h(img, top, bottom):
    draw = ImageDraw.Draw(img)
    x0 = HALF - BAND // 2
    x1 = HALF + BAND // 2
    draw.rectangle([(x0, 0 if top is None else top),
                    (x1, SIZE if bottom is None else bottom)], fill=WATER)


def draw_water_v(img, left, right):
    draw = ImageDraw.Draw(img)
    y0 = HALF - BAND // 2
    y1 = HALF + BAND // 2
    draw.rectangle([(0 if left is None else left, y0),
                    (SIZE if right is None else right, y1)], fill=WATER)


def draw_bank_top(img):
    draw = ImageDraw.Draw(img)
    draw.rectangle([(0, 0), (SIZE, BANK_BAND)], fill=BANK)


def draw_bank_bottom(img):
    draw = ImageDraw.Draw(img)
    draw.rectangle([(0, SIZE - BANK_BAND), (SIZE, SIZE)], fill=BANK)


def draw_bank_left(img):
    draw = ImageDraw.Draw(img)
    draw.rectangle([(0, 0), (BANK_BAND, SIZE)], fill=BANK)


def draw_bank_right(img):
    draw = ImageDraw.Draw(img)
    draw.rectangle([(SIZE - BANK_BAND, 0), (SIZE, SIZE)], fill=BANK)


# ---- Generators ------------------------------------------------------------

def make_water_hr():
    img = Image.new("RGBA", (SIZE, SIZE))
    fill_grass(img)
    draw_water_h(img, None, None)
    return img


def make_water_vr():
    img = Image.new("RGBA", (SIZE, SIZE))
    fill_grass(img)
    draw_water_v(img, None, None)
    return img


def make_water_corner(corner):
    """corner in {'up_l','up_r','low_l','low_r'}"""
    img = Image.new("RGBA", (SIZE, SIZE))
    fill_grass(img)
    if corner == "up_l":    # water enters from south and east -> vertical + horizontal
        draw_water_h(img, top=HALF, bottom=None)   # only the south half
        draw_water_v(img, left=HALF, right=None)   # only the east half
    elif corner == "up_r":  # water enters from south and west
        draw_water_h(img, top=HALF, bottom=None)
        draw_water_v(img, left=None, right=HALF)
    elif corner == "low_l": # water exits to south and east
        draw_water_h(img, top=None, bottom=HALF)
        draw_water_v(img, left=HALF, right=None)
    elif corner == "low_r": # water exits to south and west
        draw_water_h(img, top=None, bottom=HALF)
        draw_water_v(img, left=None, right=HALF)
    return img


def make_road_straight_bank(kind, where):
    """kind in {'hr','vr'}; where in {'top','bottom','left','right','both'}"""
    img = Image.new("RGBA", (SIZE, SIZE))
    fill_grass(img)
    if kind == "hr":
        draw_road_h(img, None, None)
    else:
        draw_road_v(img, None, None)
    if where == "top":       draw_bank_top(img)
    elif where == "bottom":  draw_bank_bottom(img)
    elif where == "left":    draw_bank_left(img)
    elif where == "right":   draw_bank_right(img)
    elif where == "both":    draw_bank_top(img); draw_bank_bottom(img)
    elif where == "lrboth":  draw_bank_left(img); draw_bank_right(img)
    return img


def make_water_straight_bank(kind, where):
    """kind in {'hr','vr'}; where in {'top','bottom','left','right','both'}"""
    img = Image.new("RGBA", (SIZE, SIZE))
    fill_grass(img)
    if kind == "hr":
        draw_water_h(img, None, None)
    else:
        draw_water_v(img, None, None)
    if where == "top":       draw_bank_top(img)
    elif where == "bottom":  draw_bank_bottom(img)
    elif where == "left":    draw_bank_left(img)
    elif where == "right":   draw_bank_right(img)
    elif where == "both":    draw_bank_top(img); draw_bank_bottom(img)
    elif where == "lrboth":  draw_bank_left(img); draw_bank_right(img)
    return img


def make_road_corner_bank(corner, where):
    """corner in {'up_l','up_r','low_l','low_r'}; where in {'top','bottom','left','right','both'}"""
    img = Image.new("RGBA", (SIZE, SIZE))
    fill_grass(img)
    # road corner
    if corner == "up_l":
        draw_road_h(img, top=HALF, bottom=None)
        draw_road_v(img, left=HALF, right=None)
    elif corner == "up_r":
        draw_road_h(img, top=HALF, bottom=None)
        draw_road_v(img, left=None, right=HALF)
    elif corner == "low_l":
        draw_road_h(img, top=None, bottom=HALF)
        draw_road_v(img, left=HALF, right=None)
    elif corner == "low_r":
        draw_road_h(img, top=None, bottom=HALF)
        draw_road_v(img, left=None, right=HALF)
    # bank on the named side(s)
    if where == "top":       draw_bank_top(img)
    elif where == "bottom":  draw_bank_bottom(img)
    elif where == "left":    draw_bank_left(img)
    elif where == "right":   draw_bank_right(img)
    elif where == "both":    draw_bank_top(img); draw_bank_left(img)  # both EMPTY edges
    elif where == "tl":      draw_bank_top(img); draw_bank_left(img)
    elif where == "tr":      draw_bank_top(img); draw_bank_right(img)
    elif where == "bl":      draw_bank_bottom(img); draw_bank_left(img)
    elif where == "br":      draw_bank_bottom(img); draw_bank_right(img)
    return img


def make_water_corner_bank(corner, where):
    img = Image.new("RGBA", (SIZE, SIZE))
    fill_grass(img)
    if corner == "up_l":
        draw_water_h(img, top=HALF, bottom=None)
        draw_water_v(img, left=HALF, right=None)
    elif corner == "up_r":
        draw_water_h(img, top=HALF, bottom=None)
        draw_water_v(img, left=None, right=HALF)
    elif corner == "low_l":
        draw_water_h(img, top=None, bottom=HALF)
        draw_water_v(img, left=HALF, right=None)
    elif corner == "low_r":
        draw_water_h(img, top=None, bottom=HALF)
        draw_water_v(img, left=None, right=HALF)
    if where == "top":       draw_bank_top(img)
    elif where == "bottom":  draw_bank_bottom(img)
    elif where == "left":    draw_bank_left(img)
    elif where == "right":   draw_bank_right(img)
    elif where == "both":    draw_bank_top(img); draw_bank_left(img)
    elif where == "tl":      draw_bank_top(img); draw_bank_left(img)
    elif where == "tr":      draw_bank_top(img); draw_bank_right(img)
    elif where == "bl":      draw_bank_bottom(img); draw_bank_left(img)
    elif where == "br":      draw_bank_bottom(img); draw_bank_right(img)
    return img


def make_grass_bank(where):
    """Grass strip with a bank on the named side."""
    img = Image.new("RGBA", (SIZE, SIZE))
    fill_grass(img)
    if where == "top":       draw_bank_top(img)
    elif where == "bottom":  draw_bank_bottom(img)
    elif where == "left":    draw_bank_left(img)
    elif where == "right":   draw_bank_right(img)
    return img


# ---- File table ------------------------------------------------------------
# Each entry: (filename, generator_function, args_tuple)

TABLE = [
    # --- Water base (6) ---
    ("water-hr.png",            make_water_hr, ()),
    ("water-vr.png",            make_water_vr, ()),
    ("water-up-l.png",          make_water_corner, ("up_l",)),
    ("water-up-r.png",          make_water_corner, ("up_r",)),
    ("water-low-l.png",         make_water_corner, ("low_l",)),
    ("water-low-r.png",         make_water_corner, ("low_r",)),

    # --- Road straight bank variants (6) ---
    ("hr-bank-top.png",         make_road_straight_bank, ("hr", "top")),
    ("hr-bank-bottom.png",      make_road_straight_bank, ("hr", "bottom")),
    ("hr-bank-both.png",        make_road_straight_bank, ("hr", "both")),
    ("vr-bank-left.png",        make_road_straight_bank, ("vr", "left")),
    ("vr-bank-right.png",       make_road_straight_bank, ("vr", "right")),
    ("vr-bank-both.png",        make_road_straight_bank, ("vr", "lrboth")),

    # --- Water straight bank variants (6) ---
    ("water-hr-bank-top.png",   make_water_straight_bank, ("hr", "top")),
    ("water-hr-bank-bottom.png",make_water_straight_bank, ("hr", "bottom")),
    ("water-hr-bank-both.png",  make_water_straight_bank, ("hr", "both")),
    ("water-vr-bank-left.png",  make_water_straight_bank, ("vr", "left")),
    ("water-vr-bank-right.png", make_water_straight_bank, ("vr", "right")),
    ("water-vr-bank-both.png",  make_water_straight_bank, ("vr", "lrboth")),

    # --- Road corner bank variants (12) ---
    ("up-l-bank-top.png",       make_road_corner_bank, ("up_l", "top")),
    ("up-l-bank-left.png",      make_road_corner_bank, ("up_l", "left")),
    ("up-l-bank-both.png",      make_road_corner_bank, ("up_l", "tl")),
    ("up-r-bank-top.png",       make_road_corner_bank, ("up_r", "top")),
    ("up-r-bank-right.png",     make_road_corner_bank, ("up_r", "right")),
    ("up-r-bank-both.png",      make_road_corner_bank, ("up_r", "tr")),
    ("low-l-bank-bottom.png",   make_road_corner_bank, ("low_l", "bottom")),
    ("low-l-bank-left.png",     make_road_corner_bank, ("low_l", "left")),
    ("low-l-bank-both.png",     make_road_corner_bank, ("low_l", "bl")),
    ("low-r-bank-bottom.png",   make_road_corner_bank, ("low_r", "bottom")),
    ("low-r-bank-right.png",    make_road_corner_bank, ("low_r", "right")),
    ("low-r-bank-both.png",     make_road_corner_bank, ("low_r", "br")),

    # --- Water corner bank variants (12) ---
    ("water-up-l-bank-top.png",      make_water_corner_bank, ("up_l", "top")),
    ("water-up-l-bank-left.png",     make_water_corner_bank, ("up_l", "left")),
    ("water-up-l-bank-both.png",     make_water_corner_bank, ("up_l", "tl")),
    ("water-up-r-bank-top.png",      make_water_corner_bank, ("up_r", "top")),
    ("water-up-r-bank-right.png",    make_water_corner_bank, ("up_r", "right")),
    ("water-up-r-bank-both.png",     make_water_corner_bank, ("up_r", "tr")),
    ("water-low-l-bank-bottom.png",  make_water_corner_bank, ("low_l", "bottom")),
    ("water-low-l-bank-left.png",    make_water_corner_bank, ("low_l", "left")),
    ("water-low-l-bank-both.png",    make_water_corner_bank, ("low_l", "bl")),
    ("water-low-r-bank-bottom.png",  make_water_corner_bank, ("low_r", "bottom")),
    ("water-low-r-bank-right.png",   make_water_corner_bank, ("low_r", "right")),
    ("water-low-r-bank-both.png",    make_water_corner_bank, ("low_r", "br")),

    # --- Grass bank tiles (4) ---
    ("grass-bank-top.png",      make_grass_bank, ("top",)),
    ("grass-bank-bottom.png",   make_grass_bank, ("bottom",)),
    ("grass-bank-left.png",     make_grass_bank, ("left",)),
    ("grass-bank-right.png",    make_grass_bank, ("right",)),
]


def main():
    if not os.path.isdir(OUT_DIR):
        os.makedirs(OUT_DIR, exist_ok=True)
    for name, gen, args in TABLE:
        img = gen(*args)
        path = os.path.join(OUT_DIR, name)
        img.save(path, "PNG")
        print("wrote", path)
    print(f"\nDone: {len(TABLE)} new tile PNGs in {OUT_DIR}")


if __name__ == "__main__":
    main()
