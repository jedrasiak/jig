#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <regex.h>

/*
gcc -Wall -Wextra -Werror src/jig-tree.c -o bin/jig-tree
find datasets/simple -type f -name "*.md" | jig-tree
find datasets/simple -type f -name "*.md" | valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes bin/jig-tree
*/

int main() {
    char filepath[PATH_MAX];
    char *pattern = "\\[.*\\]\\((.*)\\?label=parent\\)";

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
    typedef struct {
        uint src;
        uint dst;
        char *label;
    } Edge;

    Edge *edges = NULL;
    int edges_count = 0;

    // data for regex
    regex_t regex;
    regmatch_t matches[2];
    int result;

    // compile the regex pattern
    result = regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE);
    if (result != 0) {
        fprintf(stderr, "Could not compile regex\n");
        return 1;
    }

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

    // build collection of edges
    for (int i = 0; i < nodes_count; i++) {
        // execute the regex against the node content
        //printf("Testing node %d\n", nodes[i].id);
        result = regexec(&regex, nodes[i].content, 2, matches, 0);

        if (result == 0) {
            //printf("✓ Match found\n");
            //printf("%d\n", matches[1].rm_so);
            
            // grow array of edges
            Edge *tmp = realloc(edges, (edges_count + 1) * sizeof(Edge));
            if (tmp == NULL) {
                free(edges);
                fprintf(stderr, "Realloc failed\n");
                exit(EXIT_FAILURE);
            }
            edges = tmp;

            // attach data to edge
            edges[edges_count].src = nodes[i].id;
            edges[edges_count].dst = 0; // placeholder
            edges[edges_count].label = NULL; // placeholder

            edges_count++;
        } else if (result == REG_NOMATCH) {
            //printf("✗ No match\n");
        } else {
            char error_message[100];
            regerror(result, &regex, error_message, sizeof(error_message));
            fprintf(stderr, "Regex match failed: %s\n", error_message);
            return 1;
        }
    }

    // print nodes
    for (int i = 0; i < nodes_count; i++) {
        if (i == 0) {
            printf("  node address |  id |     size | path\n");
            printf("---------------+-----+----------+------------------------------\n");
        };
        printf("%p | %3d | %8ld | %s\n", (void*)&nodes[i], nodes[i].id, nodes[i].size, nodes[i].path);
    }
    printf("\n");

    // print edges
    for (int i = 0; i < edges_count; i++) {
        if (i == 0) {
            printf("  edge address | src | dst | label\n");
            printf("---------------+-----+-----+-------\n");
        };
        printf("%p | %3d | %3d | %s\n", (void*)&edges[i], edges[i].src, edges[i].dst, edges[i].label);
    }
    printf("\n");

    // print memory usage
    printf("---\n");
    printf("Node struct: %zu\n", sizeof(Node));
    printf("Node array: %zu\n", nodes_count * sizeof(Node));
    printf("Edge struct: %zu\n", sizeof(Edge));
    printf("Edge array: %zu\n", edges_count * sizeof(Edge));
    printf("---\n");

    // Free allocated memory
    for (int i = 0; i < nodes_count; i++) {
        free(nodes[i].path);
        free(nodes[i].content);
    }
    free(nodes);

    return 0;
}
