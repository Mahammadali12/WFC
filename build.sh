#!/bin/sh
set -e

gcc main.c queue.c -g -o bin/main -Wall -Wextra -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
