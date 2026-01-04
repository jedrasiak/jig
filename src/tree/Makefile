# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -I. -I..

# Output binary
TARGET = ../../bin/jig-tree

# Source files
SOURCES = tree.c tree-cli.c ../nodes/nodes.c ../edges/edges.c

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SOURCES) tree.h ../nodes/nodes.h ../edges/edges.h
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)
	@echo "Built $(TARGET)"

# Clean
clean:
	rm -f $(TARGET)
	@echo "Cleaned"

# Rebuild
rebuild: clean all

.PHONY: all clean rebuild
