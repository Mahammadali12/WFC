#!/bin/sh
set -e

if [ -z "$EMSDK" ]; then
    . ~/emsdk/emsdk_env.sh
fi

mkdir -p web

emcc src/main.c src/queue.c src/tiles.c -o web/index.html \
    -Os -Wall \
    -DPLATFORM_WEB \
    -I include \
    -I vendor/raylib-6.0-web/include \
    vendor/raylib-6.0-web/libraylib.web.a \
    -s USE_GLFW=3 \
    -s ASYNCIFY \
    -s ALLOW_MEMORY_GROWTH=1 \
    --preload-file assets/tilesets@tilesets \
    --shell-file tools/shell.html \
    -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap'] \

echo "Web build done: web/index.html (+ .js/.wasm/.data)"
