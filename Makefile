# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -I./include -I./vendor/cjson
LDFLAGS =

# Directories
SRC_DIR = src
LIB_DIR = vendor/cjson
BUILD_DIR = build
BIN_DIR = bin
INCLUDE_DIR = include

# Target executables
TARGET_JIG = $(BIN_DIR)/jig
TARGET_SEARCH = $(BIN_DIR)/jig-search-re
TARGET_TREE = $(BIN_DIR)/jig-tree

# Source files
JIG_SOURCES = $(SRC_DIR)/main.c
SEARCH_SOURCES = $(SRC_DIR)/jig-search-re.c
TREE_SOURCES = $(SRC_DIR)/jig-tree.c
LIB_SOURCES = $(wildcard $(LIB_DIR)/*.c)

# Object files
JIG_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(JIG_SOURCES))
SEARCH_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SEARCH_SOURCES))
TREE_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(TREE_SOURCES))
LIB_OBJECTS = $(patsubst $(LIB_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIB_SOURCES))

# Default target - build all executables
all: $(TARGET_JIG) $(TARGET_SEARCH) $(TARGET_TREE)

# Link jig executable
$(TARGET_JIG): $(JIG_OBJECTS) $(LIB_OBJECTS) | $(BIN_DIR)
	@$(CC) $(JIG_OBJECTS) $(LIB_OBJECTS) $(LDFLAGS) -o $(TARGET_JIG)
	@echo "LD $(TARGET_JIG)"

# Link jig-search executable
$(TARGET_SEARCH): $(SEARCH_OBJECTS) $(LIB_OBJECTS) | $(BIN_DIR)
	@$(CC) $(SEARCH_OBJECTS) $(LIB_OBJECTS) $(LDFLAGS) -o $(TARGET_SEARCH)
	@echo "LD $(TARGET_SEARCH)"

# Link jig-tree executable
$(TARGET_TREE): $(TREE_OBJECTS) | $(BIN_DIR)
	@$(CC) $(TREE_OBJECTS) $(LDFLAGS) -o $(TARGET_TREE)
	@echo "LD $(TARGET_TREE)"

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
	@rm -rf $(BUILD_DIR)/*.o $(TARGET_JIG) $(TARGET_SEARCH) $(TARGET_TREE)
	@echo "CLEAN"

# Rebuild from scratch
rebuild: clean all

# Show what would be built
info:
	@echo "Jig Sources:     $(JIG_SOURCES)"
	@echo "Search Sources:  $(SEARCH_SOURCES)"
	@echo "Tree Sources:    $(TREE_SOURCES)"
	@echo "Lib Sources:     $(LIB_SOURCES)"
	@echo "Jig Objects:     $(JIG_OBJECTS)"
	@echo "Search Objects:  $(SEARCH_OBJECTS)"
	@echo "Tree Objects:    $(TREE_OBJECTS)"
	@echo "Lib Objects:     $(LIB_OBJECTS)"
	@echo "Targets:         $(TARGET_JIG) $(TARGET_SEARCH) $(TARGET_TREE)"
	@echo "Include:         -I$(INCLUDE_DIR)"

.PHONY: all clean rebuild info
