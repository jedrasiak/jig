# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -I. -I..

# Output binary
TARGET = ../../bin/jig-note

# Source files
SOURCES = note.c note-cli.c ../uuid/uuid.c ../slugify/slugify.c ../nodes/nodes.c

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SOURCES) note.h ../uuid/uuid.h ../slugify/slugify.h ../nodes/nodes.h
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)
	@echo "Built $(TARGET)"

# Clean
clean:
	rm -f $(TARGET)
	@echo "Cleaned"

# Rebuild
rebuild: clean all

.PHONY: all clean rebuild
