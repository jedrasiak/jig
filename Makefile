# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -I./include
LDFLAGS =

# Directories
SRC_DIR = src/c
LIB_DIR = lib
BUILD_DIR = build
BIN_DIR = bin
INCLUDE_DIR = include

# Target executable
TARGET = $(BIN_DIR)/scientia

# Source files (all .c files in src/c/ and lib/)
SOURCES = $(wildcard $(SRC_DIR)/*.c)
LIB_SOURCES = $(wildcard $(LIB_DIR)/*.c)

# Object files (convert src/c/file.c to build/file.o and lib/file.c to build/file.o)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
LIB_OBJECTS = $(patsubst $(LIB_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIB_SOURCES))

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJECTS) $(LIB_OBJECTS) | $(BIN_DIR)
	@$(CC) $(OBJECTS) $(LIB_OBJECTS) $(LDFLAGS) -o $(TARGET)
	@echo "LD $(TARGET)"

# Compile .c files to .o files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "CC $<"

# Compile lib .c files to .o files
$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c | $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "CC $<"

# Create directories if they don't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean build artifacts
clean:
	@rm -rf $(BUILD_DIR)/*.o $(TARGET)
	@echo "CLEAN"

# Rebuild from scratch
rebuild: clean all

# Show what would be built
info:
	@echo "Sources:     $(SOURCES)"
	@echo "Lib Sources: $(LIB_SOURCES)"
	@echo "Objects:     $(OBJECTS)"
	@echo "Lib Objects: $(LIB_OBJECTS)"
	@echo "Target:      $(TARGET)"
	@echo "Include:     -I$(INCLUDE_DIR)"

.PHONY: all clean rebuild info
