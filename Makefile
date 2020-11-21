TARGET=csocket
CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -pedantic-errors -O2 -Isrc -Ilib
LDFLAGS=-Wl,-gc-sections -s
LIBS=-lpthread -lm
SRC_DIR=src
OBJ_DIR=obj
OUT_DIR=bin
EXTRAS=doc/ LICENSE Makefile README.md lib/thpool/thpool.*
DIST_TGZ=$(TARGET)-dist.tgz

C_FILES=$(shell find $(SRC_DIR) -type f -name '*.c') lib/thpool/thpool.c
OBJECTS=$(patsubst %.c,$(OBJ_DIR)/%.o,$(C_FILES))
HEADERS=$(shell find $(SRC_DIR) -type f -name '*.h') lib/thpool/thpool.h

.PRECIOUS: $(TARGET) $(OBJECTS)
.PHONY: default all clean
default: $(TARGET)
all: default

$(OBJ_DIR)/%.o: %.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(OUT_DIR)/$@ $(LIBS)

clean:
	@rm -rf $(OBJ_DIR) $(OUT_DIR) $(DIST_TGZ)
dist:
	@tar --create --gzip --owner=0 --group=0 --numeric-owner --xform 's:^\./::' --mtime='$(shell date -Is -d @0)' --file=$(DIST_TGZ) -- $(EXTRAS) $(SRC_DIR)
