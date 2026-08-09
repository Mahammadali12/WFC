#!/bin/sh
set -e

mkdir -p build

gcc src/main.c src/queue.c src/tiles.c -g -o build/main -Wall -Wextra -Iinclude -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
