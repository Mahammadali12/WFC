#!/usr/bin/env python3
"""
Minimal tile generator: only creates tiles that need unique art.
Bank variants reuse base PNGs (configured in main.c TILE_TEXTURE_FILES).
"""

import os
from PIL import Image

SIZE = 256
HALF = 128

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


def copy_masked(dst, src, mask):
    dp = dst.load()
    sp = src.load()
    for y in range(SIZE):
        for x in range(SIZE):
            if mask[y][x]:
                dp[x, y] = sp[x, y]


def tint_masked(dst, src, mask, tint):
    dp = dst.load()
    sp = src.load()
    tr, tg, tb = tint
    for y in range(SIZE):
        for x in range(SIZE):
            if mask[y][x]:
                sr, sg, sb, sa = sp[x, y]
                lum = int(0.299 * sr + 0.587 * sg + 0.114 * sb)
                dp[x, y] = (
                    int((tr * 0.70) + (lum * 0.30)),
                    int((tg * 0.70) + (lum * 0.30)),
                    int((tb * 0.70) + (lum * 0.30)),
                    sa,
                )


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


# -------------------------------------------------------------------------
# Generators
# -------------------------------------------------------------------------
def make_crossing():
    img = grass.copy()
    copy_masked(img, corners["up_l"], corner_masks["up_l"])
    copy_masked(img, corners["up_r"], corner_masks["up_r"])
    copy_masked(img, corners["low_l"], corner_masks["low_l"])
    copy_masked(img, corners["low_r"], corner_masks["low_r"])
    return img


def make_t_up():
    img = grass.copy()
    copy_masked(img, corners["low_l"], corner_masks["low_l"])
    copy_masked(img, corners["low_r"], corner_masks["low_r"])
    return img


def make_t_down():
    img = grass.copy()
    copy_masked(img, corners["up_l"], corner_masks["up_l"])
    copy_masked(img, corners["up_r"], corner_masks["up_r"])
    return img


def make_t_left():
    img = grass.copy()
    copy_masked(img, corners["up_r"], corner_masks["up_r"])
    copy_masked(img, corners["low_r"], corner_masks["low_r"])
    return img


def make_t_right():
    img = grass.copy()
    copy_masked(img, corners["up_l"], corner_masks["up_l"])
    copy_masked(img, corners["low_l"], corner_masks["low_l"])
    return img


def make_water_hr():
    img = grass.copy()
    tint_masked(img, hr, hr_mask, WATER_TINT)
    return img


def make_water_vr():
    img = grass.copy()
    tint_masked(img, vr, vr_mask, WATER_TINT)
    return img


def make_water_corner(corner):
    img = grass.copy()
    tint_masked(img, corners[corner], corner_masks[corner], WATER_TINT)
    return img


# -------------------------------------------------------------------------
# Build table — only tiles that need their own unique PNG
# -------------------------------------------------------------------------
TABLE = [
    ("crossing.png", make_crossing, ()),
    ("t-up.png", make_t_up, ()),
    ("t-down.png", make_t_down, ()),
    ("t-left.png", make_t_left, ()),
    ("t-right.png", make_t_right, ()),

    ("water-hr.png", make_water_hr, ()),
    ("water-vr.png", make_water_vr, ()),
    ("water-up-l.png", make_water_corner, ("up_l",)),
    ("water-up-r.png", make_water_corner, ("up_r",)),
    ("water-low-l.png", make_water_corner, ("low_l",)),
    ("water-low-r.png", make_water_corner, ("low_r",)),
]


def main():
    for name, gen, args in TABLE:
        img = gen(*args)
        save(img, name)
    print(f"\nDone: {len(TABLE)} tiles")


if __name__ == "__main__":
    main()