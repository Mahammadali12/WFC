# AGENTS.md

Wave Function Collapse road-map generator in C using raylib. Single source file (`main.c`) plus `queue.c`/`queue.h`. No tests or CI.

## Build

- Two translation units, compile directly via `./build.sh` (command lives in a comment at `main.c:7`):
  ```
  gcc main.c queue.c -g -o bin/main -Wall -Wextra -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
  ```
- Requires raylib dev headers (`raylib.h`, exposed via pkg-config). Runs from the repo root: tile textures are loaded from relative `tilesets/*.png` paths, so the working directory must be the repo.
- Build output is `bin/main` (ignored by git; `bin/` also held a stale pre-fix binary — always `./build.sh` before running). There is no Makefile.

## Architecture

- `Cell` = collapsed flag + `TileType` + `possible_tiles` bitmask (7 tiles, one bit each) + entropy. Grid is a fixed `GRID_SIZE`×`GRID_SIZE` (`main.c:10`, value 36) drawn at `CELL_SIZE` px/cell (40); window is `GRID_SIZE*CELL_SIZE` = 1440×1440.
- Tile adjacency is edge-based: `TILE_EDGES` (`main.c:43`) maps each of the 7 `TileType`s (HR, VR, UP_L, UP_R, LOW_L, LOW_R, EMPTY) to `EDGE_EMPTY`/`EDGE_ROAD` per direction. `can_be_adjacent` requires strict edge equality (edge of `tile1` facing `dir` == edge of `tile2` facing the opposite dir). Note: this tile set has **no crossing/4-way tile**, so roads can never intersect.
- `CellPos` and `Queue` (circular queue) come from `queue.h`; `propagate_cell` BFS-propagates constraints from a collapsed cell.
- Tiles are drawn from `tilesets/*.png` (256×256) via `tile_textures[]`, indexed by `TileType` (`main.c:85`). The filename→enum mapping is in `load_tile_textures`.
- `adjacency-rules.md` is the design doc but has diverged from code (doc: 10×10 grid, looser edge rule; code: 36×36, strict equality). Trust the code.

## Web build (Wasm)

- One-time toolchain: Emscripten SDK in `~/emsdk` (`git clone https://github.com/emscripten-core/emsdk.git ~/emsdk && ~/emsdk/emsdk install latest && ~/emsdk/emsdk activate latest`, then `source ~/emsdk/emsdk_env.sh` per shell; `build-web.sh` sources it automatically).
- raylib web prebuilt lives in `vendor/raylib-6.0-web/` (`libraylib.web.a`, `include/`, `shell.html` from `minshell.html` — do not delete; it is not `pkg-config`).
- Build with `./build-web.sh` → outputs `web/index.html` + `.js`/`.wasm`/`.data` (`.data` embeds `tilesets/` via `--preload-file`, so the in-code `"tilesets/*.png"` paths keep working).
- Flags are the raylib wiki standard: `-s USE_GLFW=3 -s ASYNCIFY` (needed because `main.c` uses the blocking `while(!WindowShouldClose())` loop), no desktop libs. No `main.c` changes required for web.
- Serve from the repo root (paths are relative): `python3 -m http.server 8080` → open `http://localhost:8080/web/`. Verified with headless Chromium: raylib 6.0 WEB backend boots, all 7 textures load, no console errors.

## GitHub Pages

- `.github/workflows/pages.yml` deploys the checked-in `web/` folder (static, no build step — the runner does not install Emscripten) to Pages on every push to `master`; the emcc output uses only relative paths, so `web/` content is served at the site root (`https://<user>.github.io/WFC/`).
- Before it can publish, the repo settings must have Pages → Source set to *GitHub Actions* (one-time). `web/.nojekyll` prevents Jekyll processing.
- Rebuild locally with `./build-web.sh` and commit the new `web/` files to publish an update.

## Seed drawing & controls

- The app opens in **`STATE_DRAWING`**: an empty grid. `paint_seed`/`erase_seed` maintain the drawn pattern both in the grid and in `seeds[]` (a `Seed` list), so the exact drawn pattern survives contradiction restarts via `apply_seeds()`.
- Controls (drawing): `LMB` paint hovered cell with the current brush, `RMB` cycles the brush through the 7 tiles, `MMB` or `E` erase, `R` clear all, `Enter`/`SPACE` start generation.
- Generation is `STATE_GENERATING` (~20 cells/frame). `update()` restarts on contradiction (entropy 0) via `apply_seeds()`, which re-applies the drawn seeds and propagates from each — never a blank grid. An edge-inconsistent seed pattern will restart forever; use `M` to return to drawing.
- Controls (generating): `R` regenerate from the same seeds, `M` back to drawing (WFC progress cleared, seeds kept), `SPACE` step once.

## Notes

- `porpagate` (the old misspelled, broken propagation) has been replaced by `propagate_cell`; the old `get_neighbor` no-op bug and by-value `collapse_cell` are fixed.
