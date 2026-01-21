# Compiler and flags
CC = gcc
VERSION = $(shell cat VERSION)
CFLAGS = -Wall -Wextra -Werror -I./src -I./vendor/cjson -DVERSION=\"$(VERSION)\"
LDFLAGS = -lcurl

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
VENDOR_DIR = vendor

# Target executable
TARGET = $(BIN_DIR)/jig

# Source files
SRC_SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/config/config.c $(SRC_DIR)/filter/filter.c $(SRC_DIR)/find/find.c $(SRC_DIR)/init/init.c $(SRC_DIR)/nodes/nodes.c $(SRC_DIR)/edges/edges.c $(SRC_DIR)/tree/tree.c $(SRC_DIR)/note/note.c $(SRC_DIR)/uuid/uuid.c $(SRC_DIR)/slugify/slugify.c $(SRC_DIR)/ocr/ocr.c $(SRC_DIR)/ocr/mistral.c
VENDOR_SOURCES = $(VENDOR_DIR)/cjson/cJSON.c

# Object files
SRC_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_SOURCES))
VENDOR_OBJECTS = $(patsubst $(VENDOR_DIR)/%.c,$(BUILD_DIR)/vendor/%.o,$(VENDOR_SOURCES))
OBJECTS = $(SRC_OBJECTS) $(VENDOR_OBJECTS)

# Default target
all: $(TARGET) modules

# Link executable
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $(TARGET)
	@echo "LD $(TARGET)"

# Compile src .c files to .o files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "CC $<"

# Compile vendor .c files to .o files (without -Werror for third-party code)
$(BUILD_DIR)/vendor/%.o: $(VENDOR_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@$(CC) -Wall -Wextra -I./vendor/cjson -c $< -o $@
	@echo "CC $<"

# Create directories
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Build all module binaries
modules:
	@$(MAKE) -C $(SRC_DIR)/config
	@$(MAKE) -C $(SRC_DIR)/filter
	@$(MAKE) -C $(SRC_DIR)/find
	@$(MAKE) -C $(SRC_DIR)/init
	@$(MAKE) -C $(SRC_DIR)/nodes
	@$(MAKE) -C $(SRC_DIR)/edges
	@$(MAKE) -C $(SRC_DIR)/tree
	@$(MAKE) -C $(SRC_DIR)/note
	@$(MAKE) -C $(SRC_DIR)/ocr

# Clean build artifacts
clean: clean-modules
	@rm -rf $(BUILD_DIR)
	@rm -f $(TARGET)
	@echo "CLEAN"

# Clean module binaries
clean-modules:
	@$(MAKE) -C $(SRC_DIR)/config clean
	@$(MAKE) -C $(SRC_DIR)/filter clean
	@$(MAKE) -C $(SRC_DIR)/find clean
	@$(MAKE) -C $(SRC_DIR)/init clean
	@$(MAKE) -C $(SRC_DIR)/nodes clean
	@$(MAKE) -C $(SRC_DIR)/edges clean
	@$(MAKE) -C $(SRC_DIR)/tree clean
	@$(MAKE) -C $(SRC_DIR)/note clean
	@$(MAKE) -C $(SRC_DIR)/ocr clean
	@$(MAKE) -C $(SRC_DIR)/slugify clean
	@$(MAKE) -C $(SRC_DIR)/uuid clean

# Rebuild from scratch
rebuild: clean all

.PHONY: all modules clean clean-modules rebuild
