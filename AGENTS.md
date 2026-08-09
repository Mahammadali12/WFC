# AGENTS.md

Wave Function Collapse road-map generator in C using raylib. Game logic lives in `main.c`; pure adjacency logic in `tiles.c`/`tiles.h`; BFS queue in `queue.c`/`queue.h`. Unit tests in `tests/test.c`. No CI beyond the Pages deploy workflow.

## Build

- `make` builds `bin/main`; `make run` builds + runs; `make web` rebuilds the wasm output; `make test` builds + runs the unit tests; `make tiles` regenerates the procedurally-drawn tile art; `make clean` removes `bin/`.
- The raw compile line (also in `Makefile`):
  ```
  gcc main.c queue.c tiles.c -g -o bin/main -Wall -Wextra -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
  ```
- Requires raylib dev headers (`raylib.h`, exposed via pkg-config). Runs from the repo root: tile textures are loaded from relative `tilesets/*.png` paths, so the working directory must be the repo.
- Build output is `bin/` (ignored by git; always `make` before running).

## Architecture

- `Cell` = collapsed flag + `TileType` + `possible_tiles` bitmask (**`uint16_t`, 12 tiles, one bit each**) + entropy. Grid is a fixed `GRID_SIZE`×`GRID_SIZE` (`main.c`, value 50) drawn at `CELL_SIZE` px/cell (28); window is `GRID_SIZE*CELL_SIZE` = 1400×1400.
- Tile adjacency is edge-based and lives in **`tiles.c`/`tiles.h`**: `TILE_EDGES` maps each of the 12 `TileType`s (HR, VR, 4 corners, EMPTY, CROSS, 4 T-junctions) to `EDGE_EMPTY`/`EDGE_ROAD` per direction. `can_be_adjacent` requires strict edge equality (edge of `tile1` facing `dir` == edge of `tile2` facing the opposite dir). The set includes a **crossing + 4 T-junctions**, so roads can intersect. `TILE_WEIGHTS[12]` biases `pick_weighted_tile()` (straights/empty heavy, cross/T light).
- `CellPos` and `Queue` (circular queue) come from `queue.h`; `propagate_cell` BFS-propagates constraints from a collapsed cell.
- Tiles are drawn from `tilesets/*.png` (256×256) via `tile_textures[]`, indexed by `TileType`. The filename→enum mapping is in `TILE_TEXTURE_FILES` (`main.c`).
- `adjacency-rules.md` now matches the code: 12-tile strict edge table + weights. Trust the code; regenerate art with `make tiles` if you change `tools/make_tiles.c`.
- Generation is deterministic: `find_lowest_entropy_cell` breaks ties with a hash-noise on `(x,y,wfc_seed)`, and `rand()` is re-seeded from `wfc_seed` (+ attempt counter) at each restart.

## Controls

- **Drawing** (`STATE_DRAWING`, start state):
  - `LMB` paint hovered cell with current brush; **click a tile in the top strip** to switch brush (also works with touch); `RMB` cycles brush; `MMB`/`E` erase; `R` clear; `[`/`]` step the deterministic seed; `Ctrl+Z` / `Ctrl+Shift+Z` undo/redo seed painting (snapshot stacks, depth 64); `1`–`9` load / `Ctrl+1`–`9` save a pattern slot; `P` export the current map as PNG (`wfc-map.png` in cwd, or a browser download on web); `Enter`/`SPACE` start generation.
- **Generating** (`STATE_GENERATING`): `R` restart from the same seeds, `M` back to drawing (progress cleared, seeds kept), `SPACE` step once, `[`/`]` adjust cells-per-frame (1–200, default 20). Contradictions auto-restart; after 50 consecutive restarts the **oldest seed is dropped** and generation continues (HUD notice). Uncollapsed cells are dimmed by entropy; newly collapsed cells flash white briefly.

## Patterns & persistence

- Pattern text format: `"x,y,tile;"` triplets (e.g. `3,5,0;4,5,1;`).
- Desktop: slots saved to `~/.wfc/patterns/pattern_N.wfc`. Web: slots stored in `localStorage` under `wfc_pattern_N`.
- URL param `?seeds=...` pre-populates the drawing on the web build (same triplet format).

## Web build (Wasm)

- One-time toolchain: Emscripten SDK in `~/emsdk` (`git clone https://github.com/emscripten-core/emsdk.git ~/emsdk && ~/emsdk/emsdk install latest && ~/emsdk/emsdk activate latest`, then `source ~/emsdk/emsdk_env.sh` per shell; `build-web.sh` sources it automatically).
- raylib web prebuilt lives in `vendor/raylib-6.0-web/` (`libraylib.web.a`, `include/`, `shell.html` from `minshell.html` — do not delete; it is not `pkg-config`).
- Build with `./build-web.sh` (or `make web`) → outputs `web/index.html` + `.js`/`.wasm`/`.data` (`.data` embeds `tilesets/` via `--preload-file`, so the in-code `"tilesets/*.png"` paths keep working).
- Flags: `-s USE_GLFW=3 -s ASYNCIFY` (needed because `main.c` uses the blocking `while(!WindowShouldClose())` loop), `-s ALLOW_MEMORY_GROWTH=1`, no desktop libs. `main.c` uses `#if defined(__EMSCRIPTEN__)` + `EM_JS` for the web-only pieces (localStorage, URL seeds, PNG download).
- Serve from the repo root (paths are relative): `python3 -m http.server 8080` → open `http://localhost:8080/web/`. Verify with headless Chromium: all 12 textures must load, no console errors.
- Emscripten was last verified against the `latest` emsdk install (emcc from `~/emsdk/upstream/emscripten`). If the wasm build breaks, suspect an emcc upgrade first.

## GitHub Pages

- `.github/workflows/pages.yml` deploys the checked-in `web/` folder (static, no build step — the runner does not install Emscripten) to Pages on every push to `master`; the emcc output uses only relative paths, so `web/` content is served at the site root (`https://<user>.github.io/WFC/`).
- Repo settings must have Pages → Source set to *GitHub Actions* (one-time). `web/.nojekyll` prevents Jekyll processing.
- Rebuild locally with `make web` and commit the new `web/` files to publish an update.

## Notes

- `porpagate` (the old misspelled, broken propagation) was replaced long ago by `propagate_cell`; the old `get_neighbor` no-op bug and by-value `collapse_cell` are fixed. `edges_compatible` (dead code) was removed when adjacency moved to `tiles.c`.
- `tools/make_tiles.c` is a one-shot art generator: it samples the road/grass colors and band geometry from `tilesets/horizontal-line.png` + `vertical-line.png` and renders `crossing.png` + `t-{up,down,left,right}.png`. Re-run it only to regenerate those 5 files.
