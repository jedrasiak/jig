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

    // data structure for tree node
    typedef struct {
        char id[37];
        char *path;
        long size;
        char *content;
    } Node;

    Node *nodes = NULL;
    int nodes_count = 0;

    // data structure for edge
    typedef struct {
        char src[37];
        char dst[37];
        char *label;
    } Edge;

    Edge *edges = NULL;
    int edges_count = 0;

    // data for regex
    char *rgx_link_pattern = "\\[.*\\]\\((.*)\\?label=parent\\)";
    char *rgx_id_pattern = "id: (.*)";
    regex_t rgx_link;
    regex_t rgx_id;
    regmatch_t rgx_link_matches[2];
    regmatch_t rgx_id_matches[2];
    int result;

    // compile the regex patterns
    result = regcomp(&rgx_link, rgx_link_pattern, REG_EXTENDED | REG_ICASE);
    if (result != 0) {
        fprintf(stderr, "Could not compile regex: rgx_link_pattern\n");
        return 1;
    }

    result = regcomp(&rgx_id, rgx_id_pattern, REG_EXTENDED | REG_ICASE);
    if (result != 0) {
        fprintf(stderr, "Cold not compile regex: rgx_id_pattern\n");
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
        strcpy(nodes[nodes_count].path, filepath);

        nodes_count++;
    }

    // fetch nodes content
    for (int i = 0; i < nodes_count; i++) {
        // open file
        FILE *fptr;
        if ((fptr = fopen(nodes[i].path, "r")) == NULL) {
            fprintf(stderr, "File open failed: %s\n", nodes[i].path);
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

        // read file id
        result = regexec(&rgx_id, nodes[i].content, 2, rgx_id_matches, 0);
        if (result == 0) {
            // extract matched UUID
            int match_start = rgx_id_matches[1].rm_so;
            int match_end = rgx_id_matches[1].rm_eo;
            int match_length = match_end - match_start;

            // ensure UUID fits in the buffer (36 chars + null terminator)
            if (match_length > 36) {
                match_length = 36;
            }

            // copy UUID to node
            strncpy(nodes[i].id, nodes[i].content + match_start, match_length);
            nodes[i].id[match_length] = '\0';
        } else {
            // no ID found, set empty UUID
            nodes[i].id[0] = '\0';
        }

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
        result = regexec(&rgx_link, nodes[i].content, 2, rgx_link_matches, 0);

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
            strncpy(edges[edges_count].src, nodes[i].id, 36);
            edges[edges_count].src[36] = '\0';
            strncpy(edges[edges_count].dst, "none", 36);
            edges[edges_count].dst[36] = '\0';
            edges[edges_count].label = NULL; // placeholder

            edges_count++;
        } else if (result == REG_NOMATCH) {
            //printf("✗ No match\n");
        } else {
            char error_message[100];
            regerror(result, &rgx_link, error_message, sizeof(error_message));
            fprintf(stderr, "Regex match failed: %s\n", error_message);
            return 1;
        }
    }

    // print nodes
    for (int i = 0; i < nodes_count; i++) {
        if (i == 0) {
            printf("                                uuid | path \n");
            printf("-------------------------------------+------\n");
        };
        printf("%s | %s\n", nodes[i].id, nodes[i].path);
    }
    printf("\n");

    // print edges
    for (int i = 0; i < edges_count; i++) {
        if (i == 0) {
            printf("                                 src | dst | label\n");
            printf("-------------------------------------+-----+------\n");
        };
        printf("%s | %s | %s\n", edges[i].src, edges[i].dst, edges[i].label);
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
