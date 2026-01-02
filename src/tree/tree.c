#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "tree.h"
#include "node/node.h"

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

/**
 * Add node to list (realloc for each item)
 */
static void nodelist_add(NodeList *list, const char *filepath) {
    // Grow array by one item
    Node *tmp = realloc(list->items, (list->count + 1) * sizeof(Node));
    if (tmp == NULL) {
        fprintf(stderr, "Failed to allocate memory for nodes\n");
        exit(EXIT_FAILURE);
    }
    list->items = tmp;

    // Create new node
    Node *node = &list->items[list->count];

    // Allocate and copy path
    node->path = malloc(strlen(filepath) + 1);
    if (node->path == NULL) {
        fprintf(stderr, "Failed to allocate memory for path\n");
        exit(EXIT_FAILURE);
    }
    strcpy(node->path, filepath);

    // Initialize other fields to NULL/empty
    node->id[0] = '\0';
    node->link = NULL;
    node->title = NULL;

    list->count++;
}

/**
 * Read filepaths from stdin and build node list
 */
static NodeList* process_stdin(void) {
    char filepath[PATH_MAX];

    NodeList *list = malloc(sizeof(NodeList));
    if (list == NULL) {
        fprintf(stderr, "Failed to allocate NodeList\n");
        exit(EXIT_FAILURE);
    }
    list->items = NULL;
    list->count = 0;

    while (fgets(filepath, sizeof(filepath), stdin) != NULL) {
        // Remove trailing newline
        filepath[strcspn(filepath, "\n")] = '\0';

        // Print with # prefix
        printf("# %s\n", filepath);

        // Add to node list
        nodelist_add(list, filepath);
    }

    return list;
}

/**
 * Print all nodes in the list
 */
static void print_nodes(NodeList *list) {
    printf("\n*** Nodes in list: %d ***\n\n", list->count);
    for (int i = 0; i < list->count; i++) {
        printf("%d: %s\n", i, list->items[i].path);
    }
}

/**
 * Free node list memory
 */
static void nodelist_free(NodeList *list) {
    if (list == NULL) return;

    for (int i = 0; i < list->count; i++) {
        free(list->items[i].path);
        free(list->items[i].link);
        free(list->items[i].title);
    }
    free(list->items);
    free(list);
}

int tree(int argc, char **argv) {
    // Check for help flag
    if (argc >= 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            help();
            return 0;
        }
    }

    // Process input from stdin and build node list
    NodeList *nodes = process_stdin();

    // Print the node list
    print_nodes(nodes);

    // Free memory
    nodelist_free(nodes);

    return 0;
}