#include <stdio.h>
#include <string.h>
#include "tree.h"

/**
 * Display help message
 */
static void help(void) {
    printf("Usage: jig tree [OPTIONS]\n");
    printf("       jig-tree [OPTIONS]\n");
    printf("\n");
    printf("Generate and display a tree structure of notes based on parent-child relationships.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help     Display this help and exit\n");
    printf("\n");
    printf("Examples:\n");
    printf("  jig find . -p \"\\.md$\" | jig filter | jig tree    Generate tree from markdown files\n");
}

int tree(int argc, char **argv) {
    // Check for help flag
    if (argc >= 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            help();
            return 0;
        }
    }
    return 0;
}