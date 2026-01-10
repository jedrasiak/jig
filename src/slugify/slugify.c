#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "slugify.h"

/**
 * Convert a string to a URL-friendly slug.
 *
 * Public API function. See slugify.h for full documentation.
 */
char *slugify(const char *input) {
    if (input == NULL) {
        return NULL;
    }

    // Allocate buffer (worst case: same length as input)
    size_t len = strlen(input);
    if (len == 0) {
        return strdup("");
    }

    char *buffer = malloc(len + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for slug\n");
        return NULL;
    }

    // Phase 1: Process characters
    int write_pos = 0;
    int prev_hyphen = 0;

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)input[i];

        // Skip non-ASCII characters (> 127)
        if (c > 127) {
            continue;
        }

        // Convert to lowercase
        if (c >= 'A' && c <= 'Z') {
            c = c + 32;  // to lowercase
        }

        // Check character type
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
            // Keep alphanumeric
            buffer[write_pos++] = c;
            prev_hyphen = 0;
        } else if (c == ' ' || c == '-') {
            // Space or hyphen → hyphen (but collapse multiple)
            if (!prev_hyphen && write_pos > 0) {
                buffer[write_pos++] = '-';
                prev_hyphen = 1;
            }
        }
        // All other characters: skip (remove)
    }

    // Phase 2: Trim trailing hyphen if exists
    if (write_pos > 0 && buffer[write_pos - 1] == '-') {
        write_pos--;
    }

    buffer[write_pos] = '\0';

    // Phase 3: Optimize allocation
    char *result = realloc(buffer, write_pos + 1);
    return (result != NULL) ? result : buffer;
}
