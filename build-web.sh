#!/bin/sh
set -e

if [ -z "$EMSDK" ]; then
    . ~/emsdk/emsdk_env.sh
fi

mkdir -p web

emcc main.c queue.c tiles.c -o web/index.html \
    -Os -Wall \
    -DPLATFORM_WEB \
    -I vendor/raylib-6.0-web/include \
    vendor/raylib-6.0-web/libraylib.web.a \
    -s USE_GLFW=3 \
    -s ASYNCIFY \
    -s ALLOW_MEMORY_GROWTH=1 \
    --preload-file tilesets@tilesets \
    --shell-file tools/shell.html

echo "Web build done: web/index.html (+ .js/.wasm/.data)"