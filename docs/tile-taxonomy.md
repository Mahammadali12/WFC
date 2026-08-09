# Tile Taxonomy — Road / Water / Bank System

## Summary

- **Total tiles: 53**
- Edge types: 4 (`EMPTY`, `ROAD`, `WATER`, `BANK`)
- Compatibility rule: strict edge equality
- Naming scheme: `TILE_<CATEGORY>_<SHAPE>[_BANK_<EDGES>]`
  - Categories: implicit (no prefix for road, `WATER_` for water, `GRASS_` for grass)
  - Shape: `HR`, `VR`, `UP_L`, `UP_R`, `LOW_L`, `LOW_R`
  - Bank suffix: only on the EMPTY edges that are banked
    - straights: `_BANK_TOP`, `_BANK_BOTTOM`, `_BANK_LEFT`, `_BANK_RIGHT`, `_BANK_BOTH`
      (where `_BANK_BOTH` = causeway = the two EMPTY edges both banked)
    - corners: `_BANK_TOP`, `_BANK_BOTTOM`, `_BANK_LEFT`, `_BANK_RIGHT`,
      `_BANK_BOTH` (the two EMPTY edges of that corner, both banked)

## Count breakdown

| Group | Count |
|---|---|
| Road base (HR, VR, 4 corners, EMPTY) | 7 |
| Water base (HR, VR, 4 corners) | 6 |
| Straight bank variants (3 per base straight × 4 base straights) | 12 |
| Corner bank variants (3 per base corner × 8 base corners) | 24 |
| Grass bank tiles (1 per direction) | 4 |
| **Total** | **53** |

The 53 sits inside the 40–55 sanity range from the roadmap.

## Why 53 instead of 49

The roadmap's count formula was `7 + 6 + 12 + 24 = 49`, which assumes a
grass tile can be next to a bank edge without itself having a bank edge.

That only works if we relax the strict-equality rule to "BANK matches
EMPTY", which the roadmap explicitly does not want.

Under strict equality, every cell that faces a road/water tile's BANK
edge must itself have BANK on the matching edge. The cells that need
this are grass cells sitting between a road-with-bank and a water
source. The minimum set is one bank-grass tile per direction, plus the
base grass tile — 5 grass tiles total. Four of them are the "single-edge
bank" variants; the base grass is the existing `TILE_EMPTY`.

## The full table

Format per row: `ID | Name | Top | Bottom | Left | Right | Category | Notes`

| ID  | Name                          | Top   | Bottom | Left  | Right | Cat   | Notes |
|----:|-------------------------------|-------|--------|-------|-------|-------|-------|
| 0   | TILE_HR                       | EMPTY | EMPTY  | ROAD  | ROAD  | Road  | horizontal road |
| 1   | TILE_VR                       | ROAD  | ROAD   | EMPTY | EMPTY | Road  | vertical road |
| 2   | TILE_UP_L                     | EMPTY | ROAD   | EMPTY | ROAD  | Road  | road enters from south and east |
| 3   | TILE_UP_R                     | EMPTY | ROAD   | ROAD  | EMPTY | Road  | road enters from south and west |
| 4   | TILE_LOW_L                    | ROAD  | EMPTY  | EMPTY | ROAD  | Road  | road exits to south and east |
| 5   | TILE_LOW_R                    | ROAD  | EMPTY  | ROAD  | EMPTY | Road  | road exits to south and west |
| 6   | TILE_EMPTY                    | EMPTY | EMPTY  | EMPTY | EMPTY | Grass | base grass tile |
| 7   | TILE_WATER_HR                 | EMPTY | EMPTY  | WATER | WATER | Water | horizontal river |
| 8   | TILE_WATER_VR                 | WATER | WATER  | EMPTY | EMPTY | Water | vertical river |
| 9   | TILE_WATER_UP_L               | EMPTY | WATER  | EMPTY | WATER | Water | water enters from south and east |
| 10  | TILE_WATER_UP_R               | EMPTY | WATER  | WATER | EMPTY | Water | water enters from south and west |
| 11  | TILE_WATER_LOW_L              | WATER | EMPTY  | EMPTY | WATER | Water | water exits to south and east |
| 12  | TILE_WATER_LOW_R              | WATER | EMPTY  | WATER | EMPTY | Water | water exits to south and west |
| 13  | TILE_HR_BANK_TOP              | BANK  | EMPTY  | ROAD  | ROAD  | Road  | road with bank on north side |
| 14  | TILE_HR_BANK_BOTTOM           | EMPTY | BANK   | ROAD  | ROAD  | Road  | road with bank on south side |
| 15  | TILE_HR_BANK_BOTH             | BANK  | BANK   | ROAD  | ROAD  | Road  | causeway (water both sides) |
| 16  | TILE_VR_BANK_LEFT             | ROAD  | ROAD   | BANK  | EMPTY | Road  | road with bank on west side |
| 17  | TILE_VR_BANK_RIGHT            | ROAD  | ROAD   | EMPTY | BANK  | Road  | road with bank on east side |
| 18  | TILE_VR_BANK_BOTH             | ROAD  | ROAD   | BANK  | BANK  | Road  | causeway (water both sides) |
| 19  | TILE_WATER_HR_BANK_TOP        | BANK  | EMPTY  | WATER | WATER | Water | river with bank on north side |
| 20  | TILE_WATER_HR_BANK_BOTTOM     | EMPTY | BANK   | WATER | WATER | Water | river with bank on south side |
| 21  | TILE_WATER_HR_BANK_BOTH       | BANK  | BANK   | WATER | WATER | Water | water with bank on both sides (narrow channel between roads) |
| 22  | TILE_WATER_VR_BANK_LEFT       | WATER | WATER  | BANK  | EMPTY | Water | river with bank on west side |
| 23  | TILE_WATER_VR_BANK_RIGHT      | WATER | WATER  | EMPTY | BANK  | Water | river with bank on east side |
| 24  | TILE_WATER_VR_BANK_BOTH       | WATER | WATER  | BANK  | BANK  | Water | narrow channel between roads |
| 25  | TILE_UP_L_BANK_TOP            | BANK  | ROAD   | EMPTY | ROAD  | Road  | corner with bank on north |
| 26  | TILE_UP_L_BANK_LEFT           | EMPTY | ROAD   | BANK  | ROAD  | Road  | corner with bank on west |
| 27  | TILE_UP_L_BANK_BOTH           | BANK  | ROAD   | BANK  | ROAD  | Road  | corner with bank on north+west |
| 28  | TILE_UP_R_BANK_TOP            | BANK  | ROAD   | ROAD  | EMPTY | Road  | corner with bank on north |
| 29  | TILE_UP_R_BANK_RIGHT          | EMPTY | ROAD   | ROAD  | BANK  | Road  | corner with bank on east |
| 30  | TILE_UP_R_BANK_BOTH           | BANK  | ROAD   | ROAD  | BANK  | Road  | corner with bank on north+east |
| 31  | TILE_LOW_L_BANK_BOTTOM        | ROAD  | BANK   | EMPTY | ROAD  | Road  | corner with bank on south |
| 32  | TILE_LOW_L_BANK_LEFT          | ROAD  | EMPTY  | BANK  | ROAD  | Road  | corner with bank on west |
| 33  | TILE_LOW_L_BANK_BOTH          | ROAD  | BANK   | BANK  | ROAD  | Road  | corner with bank on south+west |
| 34  | TILE_LOW_R_BANK_BOTTOM        | ROAD  | BANK   | ROAD  | EMPTY | Road  | corner with bank on south |
| 35  | TILE_LOW_R_BANK_RIGHT         | ROAD  | EMPTY  | ROAD  | BANK  | Road  | corner with bank on east |
| 36  | TILE_LOW_R_BANK_BOTH          | ROAD  | BANK   | ROAD  | BANK  | Road  | corner with bank on south+east |
| 37  | TILE_WATER_UP_L_BANK_TOP      | BANK  | WATER  | EMPTY | WATER | Water | water corner with bank on north |
| 38  | TILE_WATER_UP_L_BANK_LEFT     | EMPTY | WATER  | BANK  | WATER | Water | water corner with bank on west |
| 39  | TILE_WATER_UP_L_BANK_BOTH     | BANK  | WATER  | BANK  | WATER | Water | water corner with bank on north+west |
| 40  | TILE_WATER_UP_R_BANK_TOP      | BANK  | WATER  | WATER | EMPTY | Water | water corner with bank on north |
| 41  | TILE_WATER_UP_R_BANK_RIGHT    | EMPTY | WATER  | WATER | BANK  | Water | water corner with bank on east |
| 42  | TILE_WATER_UP_R_BANK_BOTH     | BANK  | WATER  | WATER | BANK  | Water | water corner with bank on north+east |
| 43  | TILE_WATER_LOW_L_BANK_BOTTOM  | WATER | BANK   | EMPTY | WATER | Water | water corner with bank on south |
| 44  | TILE_WATER_LOW_L_BANK_LEFT    | WATER | EMPTY  | BANK  | WATER | Water | water corner with bank on west |
| 45  | TILE_WATER_LOW_L_BANK_BOTH    | WATER | BANK   | BANK  | WATER | Water | water corner with bank on south+west |
| 46  | TILE_WATER_LOW_R_BANK_BOTTOM  | WATER | BANK   | WATER | EMPTY | Water | water corner with bank on south |
| 47  | TILE_WATER_LOW_R_BANK_RIGHT   | WATER | EMPTY  | WATER | BANK  | Water | water corner with bank on east |
| 48  | TILE_WATER_LOW_R_BANK_BOTH    | WATER | BANK   | WATER | BANK  | Water | water corner with bank on south+east |
| 49  | TILE_GRASS_BANK_TOP           | BANK  | EMPTY  | EMPTY | EMPTY | Grass | grass strip with bank to its north |
| 50  | TILE_GRASS_BANK_BOTTOM        | EMPTY | BANK   | EMPTY | EMPTY | Grass | grass strip with bank to its south |
| 51  | TILE_GRASS_BANK_LEFT          | EMPTY | EMPTY  | BANK  | EMPTY | Grass | grass strip with bank to its west |
| 52  | TILE_GRASS_BANK_RIGHT         | EMPTY | EMPTY  | EMPTY | BANK  | Grass | grass strip with bank to its east |

## Sanity checks I ran

- For every row, the 4 edges are one of {EMPTY, ROAD, WATER, BANK}. ✓
- For every road tile, the ROAD edges match the original 7-tile semantics
  on the road side. ✓
- For every water tile, the WATER edges are in the same positions as the
  road edges of the corresponding road tile (substitute ROAD → WATER). ✓
- For every bank variant, the non-banked edges are unchanged from the
  base. ✓
- Every GRASS_BANK_* tile has BANK on exactly one edge and EMPTY on the
  other three. ✓
- Every "_BANK_BOTH" tile has BANK on the two EMPTY edges of its base
  shape (so the naming "BOTH" reads naturally as "both EMPTY edges"). ✓
- TILE_COUNT = 53, sits inside the 40–55 sanity range from the roadmap. ✓

## What I did NOT do

- I did not split BANK into directional variants (e.g. BANK_OUT vs BANK_IN).
  Roadmap said to do this only if a corner-curvature case forced it; I
  worked through every corner and none forced a split. If you spot one,
  the names to split are the 24 corner `_BANK_BOTH` variants plus the 8
  base corner tiles' bank-only variants — every corner that has a
  curve on it.
- I did not add bank-on-3-edges or bank-on-all-4-edges grass tiles.
  These would be a "grass patch with banks on 3 sides" (a small water
  inlet) or "grass patch completely surrounded by water" (an island).
  Both are rare enough that the WFC can synthesise them by combining
  a 1-wide bank strip with a regular grass cell when needed. If you
  hit a propagation case that demands them, add them to the bottom
  of the table.
