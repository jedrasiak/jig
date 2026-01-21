# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -I. -I..

# Output binary
TARGET = ../../bin/jig-config

# Source files
SOURCES = config.c config-cli.c

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SOURCES) config.h
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)
	@echo "Built $(TARGET)"

# Clean
clean:
	rm -f $(TARGET)
	@echo "Cleaned"

# Rebuild
rebuild: clean all

.PHONY: all clean rebuild
