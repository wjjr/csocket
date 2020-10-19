TARGET=csocket
CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -pedantic-errors -O2
LDFLAGS=-Wl,-gc-sections -s
LIBS=-lpthread -lm -luuid -lrdkafka
SRC_DIR=src
OBJ_DIR=obj
OUT_DIR=bin
EXTRAS=doc/ LICENSE Makefile README.md
DIST_TGZ=$(TARGET)-dist.tgz

OBJECTS=$(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c $(SRC_DIR)/*/*/*.c))
HEADERS=$(wildcard $(SRC_DIR)/*.h $(SRC_DIR)/*/*.h $(SRC_DIR)/*/*/*.h)

.PRECIOUS: $(TARGET) $(OBJECTS)
.PHONY: default all clean
default: $(TARGET)
all: default

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(OUT_DIR)/$@ $(LIBS)

clean:
	@rm -rf $(OBJ_DIR) $(OUT_DIR) $(DIST_TGZ)
dist:
	@tar --create --gzip --owner=0 --group=0 --numeric-owner --xform 's:^\./::' --mtime='$(shell date -Is -d @0)' --file=$(DIST_TGZ) -- $(EXTRAS) $(SRC_DIR)
