#!/bin/sh
set -e

# Emscripten toolchain (one-time setup documented in AGENTS.md)
if [ -z "$EMSDK" ]; then
    . ~/emsdk/emsdk_env.sh
fi

mkdir -p web

emcc main.c queue.c -o web/index.html \
    -Os -Wall \
    -DPLATFORM_WEB \
    -I vendor/raylib-6.0-web/include \
    vendor/raylib-6.0-web/libraylib.web.a \
    -s USE_GLFW=3 \
    -s ASYNCIFY \
    -s ALLOW_MEMORY_GROWTH=1 \
    --preload-file tilesets@tilesets \
    --shell-file vendor/raylib-6.0-web/shell.html

echo "Web build done: web/index.html (+ .js/.wasm/.data)"