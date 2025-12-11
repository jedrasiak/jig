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
    char filepath[PATH_MAX];

    // data structure for tree node
    typedef struct {
        uint id;
        char *path;
        long size;
        char *content;
    } Node;

    Node *nodes = NULL;
    int nodes_count = 0;

    // data structure for edge
    /*
    typedef struct {
        uint src;
        uint dst;
        char *label;
    } Edge;

    Edge *edges = NULL;
    int edges_count = 0;
    */

    // build collection of nodes
    while (fgets(filepath, sizeof(filepath), stdin) != NULL) {
        // Remove trailing newline if present
        filepath[strcspn(filepath, "\n")] = '\0';

        // Grow array of nodes
        Node *tmp = realloc(nodes, (nodes_count + 1) * sizeof(Node));
        if (tmp == NULL) {
            free(nodes);
            fprintf(stderr, "Realloc failed\n");
            exit(EXIT_FAILURE);
        }
        nodes = tmp;

        // Allocate array for path
        nodes[nodes_count].path = malloc(strlen(filepath) + 1);
        if (nodes[nodes_count].path == NULL) {
            for (int i = 0; i < nodes_count; i++) {
                free(nodes[i].path);
            }
            free(nodes);
            fprintf(stderr, "Malloc failed\n");
            exit(EXIT_FAILURE);
        }

        // Attach data to node
        nodes[nodes_count].id = nodes_count;
        strcpy(nodes[nodes_count].path, filepath);

        nodes_count++;
    }

    // fetch nodes content
    for (int i = 0; i < nodes_count; i++) {
        // open file
        FILE *fptr;
        if ((fptr = fopen(nodes[i].path, "r")) == NULL) {
            fprintf(stderr, "File open failed");
            exit(EXIT_FAILURE);
        }
        
        // read file length
        fseek(fptr, 0L, SEEK_END);
        nodes[i].size = ftell(fptr);
        fseek(fptr, 0L, SEEK_SET);

        // read file content
        nodes[i].content = malloc(nodes[i].size + 1);
        if (nodes[i].content == NULL) {
            fprintf(stderr, "Malloc failed\n");
            exit(EXIT_FAILURE);
        }

        for (int j = 0; j < nodes[i].size; j++) {
            nodes[i].content[j] = fgetc(fptr);
        }
        nodes[i].content[nodes[i].size] = '\0';

        // close file
        if (fclose(fptr) != 0) {
            fprintf(stderr, "File close failed");
            exit(EXIT_FAILURE);
        }
    }

    // print nodes
    for (int i = 0; i < nodes_count; i++) {
        if (i == 0) {
            printf("       address |  id |     size | path\n");
            printf("---------------+-----+----------+------------------------------\n");
        };
        printf("%p | %3d | %8ld | %s\n", (void*)&nodes[i], nodes[i].id, nodes[i].size, nodes[i].path);
    }

    // print memory usage
    printf("---\n");
    printf("Node struct: %zu\n", sizeof(Node));
    printf("Node array: %zu\n", nodes_count * sizeof(Node));
    printf("---\n");

    // Free allocated memory
    for (int i = 0; i < nodes_count; i++) {
        free(nodes[i].path);
        free(nodes[i].content);
    }
    free(nodes);

    return 0;
}
