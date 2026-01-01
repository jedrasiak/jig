# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -I.

# Output binary
TARGET = ../../bin/jig-tree

# Source files
SOURCES = tree.c tree-cli.c

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SOURCES) tree.h
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)
	@echo "Built $(TARGET)"

# Clean
clean:
	rm -f $(TARGET)
	@echo "Cleaned"

# Rebuild
rebuild: clean all

.PHONY: all clean rebuild
