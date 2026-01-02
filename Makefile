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
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/filter/filter.c $(SRC_DIR)/find/find.c $(SRC_DIR)/tree/tree.c

# Object files (converts .c paths to .o paths in build directory)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

# Default target
all: $(TARGET)

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

# Clean build artifacts
clean:
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "CLEAN"

# Rebuild from scratch
rebuild: clean all

.PHONY: all clean rebuild
