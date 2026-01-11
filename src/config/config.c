#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Display help message
 */

static void help(void) {
    printf("Usage jig config [OPTIONS]\n");
    printf("\n");
}

int config(int argc, char **argv) {
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        }
    }
    return 0;
}