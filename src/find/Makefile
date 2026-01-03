# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -I.

# Output binary
TARGET = ../../bin/jig-find

# Source files
SOURCES = find.c find-cli.c

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SOURCES) find.h
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)
	@echo "Built $(TARGET)"

# Clean
clean:
	rm -f $(TARGET)
	@echo "Cleaned"

# Rebuild
rebuild: clean all

.PHONY: all clean rebuild
