#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

/*
gcc -Wall -Wextra -Werror src/jig-tree.c -o bin/jig-tree
find datasets/simple -type f -name "*.md" | jig-tree
find datasets/simple -type f -name "*.md" | valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes bin/jig-tree
*/

int main() {
    int items = 0;
    char filepath[PATH_MAX];

    // data structure for tree node
    typedef struct {
        uint id;
        char *path;
    } Node;

    Node *nodes = NULL;

    while (fgets(filepath, sizeof(filepath), stdin) != NULL) {
        // Remove trailing newline if present
        filepath[strcspn(filepath, "\n")] = '\0';

        // Grow array of nodes
        Node *tmp = realloc(nodes, (items + 1) * sizeof(Node));
        if (tmp == NULL) {
            free(nodes);
            fprintf(stderr, "Realloc failed\n");
            exit(EXIT_FAILURE);
        }
        nodes = tmp;

        // Allocate array for path
        nodes[items].path = malloc(strlen(filepath) + 1);
        if (nodes[items].path == NULL) {
            for (int i = 0; i < items; i++) {
                free(nodes[i].path);
            }
            free(nodes);
            fprintf(stderr, "Malloc failed\n");
            exit(EXIT_FAILURE);
        }

        // Attach data to node
        nodes[items].id = items;
        strcpy(nodes[items].path, filepath);

        items++;
    }

    for (int i = 0; i < items; i++) {
        printf("%p | %3d | %s\n", (void*)&nodes[i], nodes[i].id, nodes[i].path);
    }

    // Free allocated memory
    for (int i = 0; i < items; i++) {
        free(nodes[i].path);
    }
    free(nodes);

    printf("---\n");
    printf("Node struct: %zu\n", sizeof(Node));
    printf("Node array: %zu\n", items * sizeof(Node));
    printf("---\n");

    return 0;
}
