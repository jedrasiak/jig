# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -I. -I..

# Output binary
TARGET = ../../bin/jig-hierarchy

# Source files
SOURCES = hierarchy.c hierarchy-cli.c ../nodes/nodes.c ../edges/edges.c ../slugify/slugify.c

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SOURCES) hierarchy.h ../nodes/nodes.h ../edges/edges.h ../slugify/slugify.h
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)
	@echo "Built $(TARGET)"

# Clean
clean:
	rm -f $(TARGET)
	@echo "Cleaned"

# Rebuild
rebuild: clean all

.PHONY: all clean rebuild
