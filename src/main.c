#include <stdio.h>
#include <string.h>

#include "filter/filter.h"
#include "find/find.h"
#include "nodes/nodes.h"
#include "edges/edges.h"
#include "tree/tree.h"
#include "note/note.h"

/**
 * Display help message
 */
static void help(void) {
    printf("Usage: jig <command> [OPTIONS]\n");
    printf("\n");
    printf("Knowledge graph management tool for organizing multi-lingual content.\n");
    printf("\n");
    printf("Commands:\n");
    printf("  find                Find files in the graph\n");
    printf("  filter              Filter and parse markdown files\n");
    printf("  nodes               Manage graph nodes\n");
    printf("  edges               Manage graph edges\n");
    printf("  tree                Display hierarchical tree structure\n");
    printf("  note                Create new note scaffold\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help          Display this help and exit\n");
    printf("  -v, --version       Display version information and exit\n");
    printf("\n");
    printf("Examples:\n");
    printf("  jig find . -p \"\\.md$\" | jig filter | jig tree\n");
}

int main(int argc, char **argv) {
    
    // Check if any arguments provided
    if (argc < 2) {
        help();
        return 1;
    }

    // Check for help flag
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        help();
        return 0;
    }

    // Check for version flag
    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
        printf("jig %s\n", VERSION);
        return 0;
    }

    if (strcmp(argv[1], "filter") == 0) {
        return filter(argc - 1, argv + 1);
    }

    if (strcmp(argv[1], "find") == 0) {
        return find(argc - 1, argv + 1);
    }

    if (strcmp(argv[1], "nodes") == 0) {
        return nodes(argc - 1, argv + 1);
    }

    if (strcmp(argv[1], "edges") == 0) {
        return edges(argc - 1, argv + 1);
    }

    if (strcmp(argv[1], "tree") == 0) {
        return tree(argc - 1, argv + 1);
    }

    if (strcmp(argv[1], "note") == 0) {
        return note(argc - 1, argv + 1);
    }

    return 0;
}