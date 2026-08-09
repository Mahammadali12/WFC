CC       ?= gcc
CFLAGS   := -Wall -Wextra -Iinclude
LDFLAGS  := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRC_DIR  := src
INC_DIR  := include
TST_DIR  := tests
TL_DIR   := tools
BLD_DIR  := build

SOURCES  := $(SRC_DIR)/main.c $(SRC_DIR)/queue.c $(SRC_DIR)/tiles.c
HEADERS  := $(INC_DIR)/queue.h $(INC_DIR)/tiles.h

MAIN     := $(BLD_DIR)/main
TEST     := $(BLD_DIR)/test
GEN      := $(BLD_DIR)/make_tiles

.PHONY: all run web test tiles clean dirs

all: dirs $(MAIN)

dirs:
	mkdir -p $(BLD_DIR)

$(MAIN): $(SOURCES) $(HEADERS) | dirs
	$(CC) $(CFLAGS) -g -o $@ $(SOURCES) $(LDFLAGS)

$(GEN): $(TL_DIR)/make_tiles.c | dirs
	$(CC) $(CFLAGS) -g -o $@ $< $(LDFLAGS)

$(TEST): $(TST_DIR)/test.c $(SRC_DIR)/tiles.c $(SRC_DIR)/queue.c $(HEADERS) | dirs
	$(CC) $(CFLAGS) -g -o $@ $< $(SRC_DIR)/tiles.c $(SRC_DIR)/queue.c -lm

run: $(MAIN)
	./$(MAIN)

test: $(TEST)
	./$(TEST)

tiles: $(GEN)
	./$(GEN)

web:
	./build-web.sh

clean:
	rm -rf $(BLD_DIR)
