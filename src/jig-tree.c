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
        char *link;
        char *title;
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
    char *rgx_title_pattern = "title: (.*)";
    regex_t rgx_link;
    regex_t rgx_id;
    regex_t rgx_title;
    regmatch_t rgx_link_matches[2];
    regmatch_t rgx_id_matches[2];
    regmatch_t rgx_title_matches[2];
    int rgx_result;

    // compile the regex patterns
    rgx_result = regcomp(
        &rgx_link, 
        rgx_link_pattern, 
        REG_EXTENDED | REG_ICASE | REG_NEWLINE
    );
    if (rgx_result != 0) {
        fprintf(stderr, "Could not compile regex: rgx_link_pattern\n");
        return 1;
    }

    rgx_result = regcomp(
        &rgx_id, 
        rgx_id_pattern, 
        REG_EXTENDED | REG_ICASE | REG_NEWLINE
    );
    if (rgx_result != 0) {
        fprintf(stderr, "Cold not compile regex: rgx_id_pattern\n");
        return 1;
    }

    rgx_result = regcomp(
        &rgx_title, 
        rgx_title_pattern, 
        REG_EXTENDED | REG_ICASE | REG_NEWLINE
    );
    if (rgx_result != 0) {
        fprintf(stderr, "Cold not compile regex: rgx_title_pattern\n");
        return 1;
    }

    // build collection of nodes
    while (fgets(filepath, sizeof(filepath), stdin) != NULL) {
        long filesize;
        char *filecontent;
        
        // Remove trailing newline if present
        filepath[strcspn(filepath, "\n")] = '\0';

        // open file
        FILE *fptr;
        if ((fptr = fopen(filepath, "r")) == NULL) {
            fprintf(stderr, "File open failed: %s\n", filepath);
            exit(EXIT_FAILURE);
        }
        
        // read file length
        fseek(fptr, 0L, SEEK_END);
        filesize = ftell(fptr);
        fseek(fptr, 0L, SEEK_SET);

        // read file content
        filecontent = malloc(filesize + 1);
        if (filecontent == NULL) {
            fprintf(stderr, "Malloc failed\n");
            exit(EXIT_FAILURE);
        }

        for (int j = 0; j < filesize; j++) {
            filecontent[j] = fgetc(fptr);
        }
        filecontent[filesize] = '\0';

        // read file id
        rgx_result = regexec(&rgx_id, filecontent, 2, rgx_id_matches, 0);

        // skip if no id
        if (rgx_result == REG_NOMATCH) {
            free(filecontent);
            continue;
        }

        // Grow array of nodes
        Node *tmp = realloc(nodes, (nodes_count + 1) * sizeof(Node));
        if (tmp == NULL) {
            free(nodes);
            fprintf(stderr, "Realloc failed\n");
            exit(EXIT_FAILURE);
        }
        nodes = tmp;

        // extract matched id
        int match_start = rgx_id_matches[1].rm_so;
        int match_end = rgx_id_matches[1].rm_eo;
        int match_length = match_end - match_start;

        // ensure id fits in the buffer (36 chars + null terminator)
        if (match_length > 36) {
            match_length = 36;
        }

        // attach id to node
        strncpy(nodes[nodes_count].id, filecontent + match_start, match_length);
        nodes[nodes_count].id[match_length] = '\0';

        // read link in file
        rgx_result = regexec(&rgx_link, filecontent, 2, rgx_link_matches, 0);

        if (rgx_result == 0) {
            // extract matched id
            int match_start = rgx_link_matches[1].rm_so;
            int match_end = rgx_link_matches[1].rm_eo;
            int match_length = match_end - match_start;

            // allocate array for link
            nodes[nodes_count].link = malloc(match_length + 1);
            if (nodes[nodes_count].link == NULL) {
                for (int i = 0; i < nodes_count; i++) {
                    free(nodes[i].path);
                    free(nodes[i].link);
                    free(nodes[i].title);
                }
                free(nodes);
                fprintf(stderr, "Malloc failed\n");
                exit(EXIT_FAILURE);
            }

            // attach link to node
            strncpy(nodes[nodes_count].link, filecontent + match_start, match_length);
            nodes[nodes_count].link[match_length] = '\0';
        } else {
            nodes[nodes_count].link = NULL;
        }

        // read title in file
        rgx_result = regexec(&rgx_title, filecontent, 2, rgx_title_matches, 0);

        if (rgx_result == 0) {
            // extract matched title
            int match_start = rgx_title_matches[1].rm_so;
            int match_end = rgx_title_matches[1].rm_eo;
            int match_length = match_end - match_start;

            // allocate array for title
            nodes[nodes_count].title = malloc(match_length + 1);
            if (nodes[nodes_count].title == NULL) {
                for (int i = 0; i < nodes_count; i++) {
                    free(nodes[i].path);
                    free(nodes[i].link);
                    free(nodes[i].title);
                }
                free(nodes);
                fprintf(stderr, "Malloc failed\n");
                exit(EXIT_FAILURE);
            }

            // attach title to node
            strncpy(nodes[nodes_count].title, filecontent + match_start, match_length);
            nodes[nodes_count].title[match_length] = '\0';
        } else {
            nodes[nodes_count].title = NULL;
        }

        // Allocate array for path
        nodes[nodes_count].path = malloc(strlen(filepath) + 1);
        if (nodes[nodes_count].path == NULL) {
            for (int i = 0; i < nodes_count; i++) {
                free(nodes[i].path);
                free(nodes[i].link);
                free(nodes[i].title);
            }
            free(nodes);
            fprintf(stderr, "Malloc failed\n");
            exit(EXIT_FAILURE);
        }

        // Attach path to node
        strcpy(nodes[nodes_count].path, filepath);

        // close file
        if (fclose(fptr) != 0) {
            fprintf(stderr, "File close failed");
            exit(EXIT_FAILURE);
        }

        // free file content
        free(filecontent);

        // increment nodes count
        nodes_count++;
    }


    /*
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
    */

    // print nodes
    printf("*** nodes ***\n\n");
    for (int i = 0; i < nodes_count; i++) {
        printf("id: %s\n", nodes[i].id);
        printf("path: %s\n", nodes[i].path);
        printf("link: %s\n", nodes[i].link);
        printf("title: %s\n", nodes[i].title);
        printf("***\n");
    }
    printf("\n");

    /*
    // print edges
    for (int i = 0; i < edges_count; i++) {
        if (i == 0) {
            printf("                                 src | dst | label\n");
            printf("-------------------------------------+-----+------\n");
        };
        printf("%s | %s | %s\n", edges[i].src, edges[i].dst, edges[i].label);
    }
    printf("\n");
    */

    // print memory usage
    printf("*** memory ***\n\n");
    printf("Node struct: %zu\n", sizeof(Node));
    printf("Node array: %zu\n", nodes_count * sizeof(Node));
    //printf("Edge struct: %zu\n", sizeof(Edge));
    //printf("Edge array: %zu\n", edges_count * sizeof(Edge));

    // Free allocated memory
    for (int i = 0; i < nodes_count; i++) {
        free(nodes[i].path);
        free(nodes[i].link);
        free(nodes[i].title);
    }
    free(nodes);

    // Free compiled regex patterns
    regfree(&rgx_link);
    regfree(&rgx_id);
    regfree(&rgx_title);

    return 0;
}
