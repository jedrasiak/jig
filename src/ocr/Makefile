# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -I. -I.. -I../../vendor/cjson
LDFLAGS = -lcurl

# Output binary
TARGET = ../../bin/jig-ocr

# Source files
SOURCES = ocr.c ocr-cli.c mistral.c ../config/config.c ../../vendor/cjson/cJSON.c

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SOURCES) ocr.h mistral.h
	$(CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $(TARGET)
	@echo "Built $(TARGET)"

# Clean
clean:
	rm -f $(TARGET)
	@echo "Cleaned"

# Rebuild
rebuild: clean all

.PHONY: all clean rebuild
