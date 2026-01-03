# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -I. -I..

# Output binary
TARGET = ../../bin/jig-edges

# Source files
SOURCES = edges.c edges-cli.c ../nodes/nodes.c

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SOURCES) edges.h ../nodes/nodes.h
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)
	@echo "Built $(TARGET)"

# Clean
clean:
	rm -f $(TARGET)
	@echo "Cleaned"

# Rebuild
rebuild: clean all

.PHONY: all clean rebuild
