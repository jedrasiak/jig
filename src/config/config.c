#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"

static void help(void);
static void parse(const char *filepath);

int config(int argc, char **argv) {
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        } else {
            parse(argv[i]);
            return 0;
        }
    }
    return 0;
}

/* --- */
static void help(void) {
    printf("Usage jig config [OPTIONS]\n");
    printf("\n");
}

static void parse(const char *filepath) {
    printf("Parsing config file: %s\n", filepath);

    FILE *fptr = fopen(filepath, "r");
    if (fptr == NULL) {
        fprintf(stderr, "Error: Unable to open config file %s\n", filepath);
        return;
    }

    char line[MAX_LINE];
    char section[MAX_KEY] = "";

    while (fgets(line, sizeof(line), fptr)) {
        // Trim newline
        line[strcspn(line, "\n")] = 0;

        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\0') {
            continue;
        }

        // Check for section header
        if (line[0] == '[') {
            char *end = strchr(line, ']');
            if (end) {
                size_t len = end - line - 1;
                strncpy(section, line + 1, len);
                section[len] = '\0';
            }
            continue;
        }

        // Parse key=value
        char *equals = strchr(line, '=');
        if (equals) {
            *equals = '\0';
            char *key = line;
            char *value = equals + 1;

            // Trim whitespace
            while (*key == ' ') key++;
            while (*value == ' ') value++;

            printf("Section: [%s] Key: '%s' Value: '%s'\n", section, key, value);
        }
    }

    fclose(fptr);
}