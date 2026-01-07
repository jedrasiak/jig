# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -I./src
LDFLAGS =

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Target executable
TARGET = $(BIN_DIR)/jig

# Source files
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/filter/filter.c $(SRC_DIR)/find/find.c $(SRC_DIR)/nodes/nodes.c $(SRC_DIR)/edges/edges.c $(SRC_DIR)/tree/tree.c $(SRC_DIR)/note/note.c $(SRC_DIR)/uuid/uuid.c $(SRC_DIR)/slugify/slugify.c

# Object files (converts .c paths to .o paths in build directory)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

# Default target
all: $(TARGET) modules

# Link executable
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $(TARGET)
	@echo "LD $(TARGET)"

# Compile .c files to .o files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "CC $<"

# Create directories
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Build all module binaries
modules:
	@$(MAKE) -C $(SRC_DIR)/filter
	@$(MAKE) -C $(SRC_DIR)/find
	@$(MAKE) -C $(SRC_DIR)/nodes
	@$(MAKE) -C $(SRC_DIR)/edges
	@$(MAKE) -C $(SRC_DIR)/tree
	@$(MAKE) -C $(SRC_DIR)/note

# Clean build artifacts
clean: clean-modules
	@rm -rf $(BUILD_DIR)
	@rm -f $(TARGET)
	@echo "CLEAN"

# Clean module binaries
clean-modules:
	@$(MAKE) -C $(SRC_DIR)/filter clean
	@$(MAKE) -C $(SRC_DIR)/find clean
	@$(MAKE) -C $(SRC_DIR)/nodes clean
	@$(MAKE) -C $(SRC_DIR)/edges clean
	@$(MAKE) -C $(SRC_DIR)/tree clean
	@$(MAKE) -C $(SRC_DIR)/note clean

# Rebuild from scratch
rebuild: clean all

.PHONY: all modules clean clean-modules rebuild
