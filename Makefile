CC       ?= gcc
CFLAGS   := -Wall -Wextra
LDFLAGS  := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SOURCES  := main.c queue.c tiles.c
HEADERS  := queue.h tiles.h

BIN      := bin
MAIN     := $(BIN)/main
TEST     := $(BIN)/test
GEN      := $(BIN)/make_tiles

.PHONY: all run web test tiles clean

all: $(MAIN)

$(BIN):
	mkdir -p $(BIN)

$(MAIN): $(SOURCES) $(HEADERS) | $(BIN)
	$(CC) $(CFLAGS) -g -o $@ $(SOURCES) $(LDFLAGS)

$(GEN): tools/make_tiles.c | $(BIN)
	$(CC) $(CFLAGS) -g -o $@ tools/make_tiles.c $(LDFLAGS)

$(TEST): tests/test.c tiles.c queue.c | $(BIN)
	$(CC) $(CFLAGS) -g -I. -o $@ tests/test.c tiles.c queue.c -lm

run: $(MAIN)
	./$(MAIN)

test: $(TEST)
	./$(TEST)

tiles: $(GEN)
	./$(GEN)

web:
	./build-web.sh

clean:
	rm -rf $(BIN)
