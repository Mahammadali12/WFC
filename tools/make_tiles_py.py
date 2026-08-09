#!/usr/bin/env python3
"""
Procedural tile art generator that matches the existing hand-drawn tileset.
Composites strips from actual tileset PNGs instead of drawing solid colors.
Also generates crossing + T-junctions (replacing the old solid-color C tool).
"""

import os
from PIL import Image

SIZE = 256
HALF = 128
BANK_BAND = 22

BASE_DIR = os.path.join(os.path.dirname(__file__), "..", "tilesets")


def load(name):
    return Image.open(os.path.join(BASE_DIR, name)).convert("RGBA")


def save(img, name):
    img.save(os.path.join(BASE_DIR, name), "PNG")
    print("wrote", name)


def extract_road_mask(src_img, grass_img):
    src = src_img.load()
    grass = grass_img.load()
    mask = [[False] * SIZE for _ in range(SIZE)]
    for y in range(SIZE):
        for x in range(SIZE):
            sr, sg, sb, _ = src[x, y]
            gr, gg, gb, _ = grass[x, y]
            if (sr - gr) ** 2 + (sg - gg) ** 2 + (sb - gb) ** 2 > 3000:
                mask[y][x] = True
    return mask


def copy_masked(dst, src, mask, x0=0, y0=0, x1=SIZE, y1=SIZE):
    dp = dst.load()
    sp = src.load()
    for y in range(max(0, y0), min(SIZE, y1)):
        for x in range(max(0, x0), min(SIZE, x1)):
            if mask[y][x]:
                dp[x, y] = sp[x, y]


def tint_masked(dst, src, mask, tint, x0=0, y0=0, x1=SIZE, y1=SIZE):
    dp = dst.load()
    sp = src.load()
    tr, tg, tb = tint
    for y in range(max(0, y0), min(SIZE, y1)):
        for x in range(max(0, x0), min(SIZE, x1)):
            if mask[y][x]:
                sr, sg, sb, sa = sp[x, y]
                lum = int(0.299 * sr + 0.587 * sg + 0.114 * sb)
                dp[x, y] = (
                    int((tr * 0.70) + (lum * 0.30)),
                    int((tg * 0.70) + (lum * 0.30)),
                    int((tb * 0.70) + (lum * 0.30)),
                    sa,
                )


def fill_band(dst, color, x0, y0, x1, y1):
    draw = Image.new("RGBA", (x1 - x0, y1 - y0), color)
    dst.paste(draw, (x0, y0))


def bank_top(dst):
    fill_band(dst, BANK_COLOR, 0, 0, SIZE, BANK_BAND)


def bank_bottom(dst):
    fill_band(dst, BANK_COLOR, 0, SIZE - BANK_BAND, SIZE, SIZE)


def bank_left(dst):
    fill_band(dst, BANK_COLOR, 0, 0, BANK_BAND, SIZE)


def bank_right(dst):
    fill_band(dst, BANK_COLOR, SIZE - BANK_BAND, 0, SIZE, SIZE)


# -------------------------------------------------------------------------
# Load source art
# -------------------------------------------------------------------------
grass = load("empty-green.png")
hr = load("horizontal-line.png")
vr = load("vertical-line.png")

corners = {
    "up_l": load("upper-left-corner.png"),
    "up_r": load("upper-right-corner.png"),
    "low_l": load("lower-left-corner.png"),
    "low_r": load("lower-right-corner.png"),
}

hr_mask = extract_road_mask(hr, grass)
vr_mask = extract_road_mask(vr, grass)
corner_masks = {k: extract_road_mask(v, grass) for k, v in corners.items()}

WATER_TINT = (62, 102, 140)
BANK_COLOR = (180, 158, 110, 255)


# -------------------------------------------------------------------------
# Straight-road helpers
# -------------------------------------------------------------------------
def road_h(dst, left=None, right=None):
    """Horizontal road band. Crop horizontally with left/right."""
    x0 = 0 if left is None else left
    x1 = SIZE if right is None else right
    copy_masked(dst, hr, hr_mask, x0, 0, x1, SIZE)


def road_v(dst, top=None, bottom=None):
    """Vertical road band. Crop vertically with top/bottom."""
    y0 = 0 if top is None else top
    y1 = SIZE if bottom is None else bottom
    copy_masked(dst, vr, vr_mask, 0, y0, SIZE, y1)


def water_h(dst, left=None, right=None):
    x0 = 0 if left is None else left
    x1 = SIZE if right is None else right
    tint_masked(dst, hr, hr_mask, WATER_TINT, x0, 0, x1, SIZE)


def water_v(dst, top=None, bottom=None):
    y0 = 0 if top is None else top
    y1 = SIZE if bottom is None else bottom
    tint_masked(dst, vr, vr_mask, WATER_TINT, 0, y0, SIZE, y1)


# -------------------------------------------------------------------------
# Tile generators
# -------------------------------------------------------------------------
def make_crossing():
    img = grass.copy()
    road_h(img)
    road_v(img)
    return img


def make_t_up():
    img = grass.copy()
    road_h(img)
    road_v(img, top=0, bottom=HALF)
    return img


def make_t_down():
    img = grass.copy()
    road_h(img)
    road_v(img, top=HALF, bottom=SIZE)
    return img


def make_t_left():
    img = grass.copy()
    road_v(img)
    road_h(img, left=0, right=HALF)
    return img


def make_t_right():
    img = grass.copy()
    road_v(img)
    road_h(img, left=HALF, right=SIZE)
    return img


def make_water_hr():
    img = grass.copy()
    water_h(img)
    return img


def make_water_vr():
    img = grass.copy()
    water_v(img)
    return img


def make_water_corner(corner):
    img = grass.copy()
    tint_masked(img, corners[corner], corner_masks[corner], WATER_TINT)
    return img


def make_road_straight_bank(kind, where):
    img = grass.copy()
    if kind == "hr":
        road_h(img)
    else:
        road_v(img)

    if where == "top":
        bank_top(img)
    elif where == "bottom":
        bank_bottom(img)
    elif where == "left":
        bank_left(img)
    elif where == "right":
        bank_right(img)
    elif where == "both":
        bank_top(img)
        bank_bottom(img)
    elif where == "lrboth":
        bank_left(img)
        bank_right(img)
    return img


def make_water_straight_bank(kind, where):
    img = grass.copy()
    if kind == "hr":
        water_h(img)
    else:
        water_v(img)

    if where == "top":
        bank_top(img)
    elif where == "bottom":
        bank_bottom(img)
    elif where == "left":
        bank_left(img)
    elif where == "right":
        bank_right(img)
    elif where == "both":
        bank_top(img)
        bank_bottom(img)
    elif where == "lrboth":
        bank_left(img)
        bank_right(img)
    return img


def make_road_corner_bank(corner, where):
    img = grass.copy()
    copy_masked(img, corners[corner], corner_masks[corner])

    if where == "top":
        bank_top(img)
    elif where == "bottom":
        bank_bottom(img)
    elif where == "left":
        bank_left(img)
    elif where == "right":
        bank_right(img)
    elif where in ("both", "tl"):
        bank_top(img)
        bank_left(img)
    elif where == "tr":
        bank_top(img)
        bank_right(img)
    elif where == "bl":
        bank_bottom(img)
        bank_left(img)
    elif where == "br":
        bank_bottom(img)
        bank_right(img)
    return img


def make_water_corner_bank(corner, where):
    img = grass.copy()
    tint_masked(img, corners[corner], corner_masks[corner], WATER_TINT)

    if where == "top":
        bank_top(img)
    elif where == "bottom":
        bank_bottom(img)
    elif where == "left":
        bank_left(img)
    elif where == "right":
        bank_right(img)
    elif where in ("both", "tl"):
        bank_top(img)
        bank_left(img)
    elif where == "tr":
        bank_top(img)
        bank_right(img)
    elif where == "bl":
        bank_bottom(img)
        bank_left(img)
    elif where == "br":
        bank_bottom(img)
        bank_right(img)
    return img


def make_grass_bank(where):
    img = grass.copy()
    if where == "top":
        bank_top(img)
    elif where == "bottom":
        bank_bottom(img)
    elif where == "left":
        bank_left(img)
    elif where == "right":
        bank_right(img)
    return img


# -------------------------------------------------------------------------
# Build table
# -------------------------------------------------------------------------
TABLE = [
    # Crossing + T-junctions (textured, replacing old solid-color C tool)
    ("crossing.png", make_crossing, ()),
    ("t-up.png", make_t_up, ()),
    ("t-down.png", make_t_down, ()),
    ("t-left.png", make_t_left, ()),
    ("t-right.png", make_t_right, ()),

    # Water base
    ("water-hr.png", make_water_hr, ()),
    ("water-vr.png", make_water_vr, ()),
    ("water-up-l.png", make_water_corner, ("up_l",)),
    ("water-up-r.png", make_water_corner, ("up_r",)),
    ("water-low-l.png", make_water_corner, ("low_l",)),
    ("water-low-r.png", make_water_corner, ("low_r",)),

    # Road straight bank
    ("hr-bank-top.png", make_road_straight_bank, ("hr", "top")),
    ("hr-bank-bottom.png", make_road_straight_bank, ("hr", "bottom")),
    ("hr-bank-both.png", make_road_straight_bank, ("hr", "both")),
    ("vr-bank-left.png", make_road_straight_bank, ("vr", "left")),
    ("vr-bank-right.png", make_road_straight_bank, ("vr", "right")),
    ("vr-bank-both.png", make_road_straight_bank, ("vr", "lrboth")),

    # Water straight bank
    ("water-hr-bank-top.png", make_water_straight_bank, ("hr", "top")),
    ("water-hr-bank-bottom.png", make_water_straight_bank, ("hr", "bottom")),
    ("water-hr-bank-both.png", make_water_straight_bank, ("hr", "both")),
    ("water-vr-bank-left.png", make_water_straight_bank, ("vr", "left")),
    ("water-vr-bank-right.png", make_water_straight_bank, ("vr", "right")),
    ("water-vr-bank-both.png", make_water_straight_bank, ("vr", "lrboth")),

    # Road corner bank
    ("up-l-bank-top.png", make_road_corner_bank, ("up_l", "top")),
    ("up-l-bank-left.png", make_road_corner_bank, ("up_l", "left")),
    ("up-l-bank-both.png", make_road_corner_bank, ("up_l", "tl")),
    ("up-r-bank-top.png", make_road_corner_bank, ("up_r", "top")),
    ("up-r-bank-right.png", make_road_corner_bank, ("up_r", "right")),
    ("up-r-bank-both.png", make_road_corner_bank, ("up_r", "tr")),
    ("low-l-bank-bottom.png", make_road_corner_bank, ("low_l", "bottom")),
    ("low-l-bank-left.png", make_road_corner_bank, ("low_l", "left")),
    ("low-l-bank-both.png", make_road_corner_bank, ("low_l", "bl")),
    ("low-r-bank-bottom.png", make_road_corner_bank, ("low_r", "bottom")),
    ("low-r-bank-right.png", make_road_corner_bank, ("low_r", "right")),
    ("low-r-bank-both.png", make_road_corner_bank, ("low_r", "br")),

    # Water corner bank
    ("water-up-l-bank-top.png", make_water_corner_bank, ("up_l", "top")),
    ("water-up-l-bank-left.png", make_water_corner_bank, ("up_l", "left")),
    ("water-up-l-bank-both.png", make_water_corner_bank, ("up_l", "tl")),
    ("water-up-r-bank-top.png", make_water_corner_bank, ("up_r", "top")),
    ("water-up-r-bank-right.png", make_water_corner_bank, ("up_r", "right")),
    ("water-up-r-bank-both.png", make_water_corner_bank, ("up_r", "tr")),
    ("water-low-l-bank-bottom.png", make_water_corner_bank, ("low_l", "bottom")),
    ("water-low-l-bank-left.png", make_water_corner_bank, ("low_l", "left")),
    ("water-low-l-bank-both.png", make_water_corner_bank, ("low_l", "bl")),
    ("water-low-r-bank-bottom.png", make_water_corner_bank, ("low_r", "bottom")),
    ("water-low-r-bank-right.png", make_water_corner_bank, ("low_r", "right")),
    ("water-low-r-bank-both.png", make_water_corner_bank, ("low_r", "br")),

    # Grass bank
    ("grass-bank-top.png", make_grass_bank, ("top",)),
    ("grass-bank-bottom.png", make_grass_bank, ("bottom",)),
    ("grass-bank-left.png", make_grass_bank, ("left",)),
    ("grass-bank-right.png", make_grass_bank, ("right",)),
]


def main():
    for name, gen, args in TABLE:
        img = gen(*args)
        save(img, name)
    print(f"\nDone: {len(TABLE)} tiles")


if __name__ == "__main__":
    main()