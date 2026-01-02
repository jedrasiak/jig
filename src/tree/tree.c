#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <regex.h>

#include "tree.h"
#include "node/node.h"
#include "edge/edge.h"

// Global compiled regexes
static regex_t rgx_link;
static regex_t rgx_id;
static regex_t rgx_title;
static int regexes_compiled = 0;

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
    printf("  -h, --help          Display this help and exit\n");
    printf("  -f, --format FORMAT Output format (md for markdown, csv for CSV)\n");
    printf("\n");
    printf("Examples:\n");
    printf("  jig find . -p \"\\.md$\" | jig filter | jig tree        Generate tree from markdown files\n");
    printf("  jig find . -p \"\\.md$\" | jig filter | jig tree -f md   Output in markdown format\n");
    printf("  jig find . -p \"\\.md$\" | jig filter | jig tree -f csv  Output in CSV format\n");
}

/**
 * Compile regex patterns for parsing node fields
 */
static void compile_regexes(void) {
    if (regexes_compiled) return;

    char *rgx_link_pattern = "\\[.*\\]\\((.*)\\?label=parent\\)";
    char *rgx_id_pattern = "id: (.*)";
    char *rgx_title_pattern = "title: (.*)";
    int rgx_result;

    rgx_result = regcomp(&rgx_link, rgx_link_pattern, REG_EXTENDED | REG_ICASE | REG_NEWLINE);
    if (rgx_result != 0) {
        fprintf(stderr, "Could not compile regex: rgx_link_pattern\n");
        exit(EXIT_FAILURE);
    }

    rgx_result = regcomp(&rgx_id, rgx_id_pattern, REG_EXTENDED | REG_ICASE | REG_NEWLINE);
    if (rgx_result != 0) {
        fprintf(stderr, "Could not compile regex: rgx_id_pattern\n");
        exit(EXIT_FAILURE);
    }

    rgx_result = regcomp(&rgx_title, rgx_title_pattern, REG_EXTENDED | REG_ICASE | REG_NEWLINE);
    if (rgx_result != 0) {
        fprintf(stderr, "Could not compile regex: rgx_title_pattern\n");
        exit(EXIT_FAILURE);
    }

    regexes_compiled = 1;
}

/**
 * Free compiled regex patterns
 */
static void free_regexes(void) {
    if (!regexes_compiled) return;

    regfree(&rgx_link);
    regfree(&rgx_id);
    regfree(&rgx_title);

    regexes_compiled = 0;
}

/**
 * Parse node fields (id, title, link) from file content
 */
static void parse_node(Node *node, const char *filepath) {
    // Initialize fields to NULL/empty
    node->id[0] = '\0';
    node->link = NULL;
    node->title = NULL;

    FILE *fptr = fopen(filepath, "r");
    if (fptr == NULL) {
        return;  // Skip file if can't open
    }

    // Read file size
    fseek(fptr, 0L, SEEK_END);
    long filesize = ftell(fptr);
    fseek(fptr, 0L, SEEK_SET);

    // Read file content
    char *filecontent = malloc(filesize + 1);
    if (filecontent == NULL) {
        fclose(fptr);
        return;
    }

    for (long j = 0; j < filesize; j++) {
        filecontent[j] = fgetc(fptr);
    }
    filecontent[filesize] = '\0';
    fclose(fptr);

    // Parse ID
    regmatch_t rgx_id_matches[2];
    int rgx_result = regexec(&rgx_id, filecontent, 2, rgx_id_matches, 0);
    if (rgx_result == 0) {
        int match_start = rgx_id_matches[1].rm_so;
        int match_end = rgx_id_matches[1].rm_eo;
        int match_length = match_end - match_start;
        if (match_length > 36) {
            match_length = 36;
        }
        strncpy(node->id, filecontent + match_start, match_length);
        node->id[match_length] = '\0';
    }

    // Parse link
    regmatch_t rgx_link_matches[2];
    rgx_result = regexec(&rgx_link, filecontent, 2, rgx_link_matches, 0);
    if (rgx_result == 0) {
        int match_start = rgx_link_matches[1].rm_so;
        int match_end = rgx_link_matches[1].rm_eo;
        int match_length = match_end - match_start;

        node->link = malloc(match_length + 1);
        if (node->link != NULL) {
            strncpy(node->link, filecontent + match_start, match_length);
            node->link[match_length] = '\0';
        }
    }

    // Parse title
    regmatch_t rgx_title_matches[2];
    rgx_result = regexec(&rgx_title, filecontent, 2, rgx_title_matches, 0);
    if (rgx_result == 0) {
        int match_start = rgx_title_matches[1].rm_so;
        int match_end = rgx_title_matches[1].rm_eo;
        int match_length = match_end - match_start;

        node->title = malloc(match_length + 1);
        if (node->title != NULL) {
            strncpy(node->title, filecontent + match_start, match_length);
            node->title[match_length] = '\0';
        }
    }

    free(filecontent);
}

/**
 * Add node to list (realloc for each item)
 */
static void add_node(NodeList *list, const char *filepath) {
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

    // Parse node fields from file
    parse_node(node, filepath);

    list->count++;
}

/**
 * Read filepaths from stdin and build node list
 */
static NodeList* build_nodes(void) {
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

        // Add to node list
        add_node(list, filepath);
    }

    return list;
}

/**
 * Print all nodes in the list
 */
static void print_nodes(NodeList *list, const char *format) {
    if (format != NULL && strcmp(format, "md") == 0) {
        // Markdown format
        for (int i = 0; i < list->count; i++) {
            const char *title = list->items[i].title ? list->items[i].title : "title";
            printf("[%s](%s)\n", title, list->items[i].path);
        }
    } else if (format != NULL && strcmp(format, "csv") == 0) {
        // CSV format
        printf("id,title,path,link\n");
        for (int i = 0; i < list->count; i++) {
            printf("%s,%s,%s,%s\n",
                   list->items[i].id[0] ? list->items[i].id : "",
                   list->items[i].title ? list->items[i].title : "",
                   list->items[i].path ? list->items[i].path : "",
                   list->items[i].link ? list->items[i].link : "");
        }
    } else {
        // Default format
        for (int i = 0; i < list->count; i++) {
            printf("%d: %s\n", i, list->items[i].path);
        }
    }
}

/**
 * Free node list memory
 */
static void free_nodes(NodeList *list) {
    if (list == NULL) return;

    for (int i = 0; i < list->count; i++) {
        free(list->items[i].path);
        free(list->items[i].link);
        free(list->items[i].title);
    }
    free(list->items);
    free(list);
}

/**
 * Build edges from nodes by matching links to paths
 */
static EdgeList* build_edges(NodeList *nodes) {
    EdgeList *edges = malloc(sizeof(EdgeList));
    if (edges == NULL) {
        fprintf(stderr, "Failed to allocate EdgeList\n");
        exit(EXIT_FAILURE);
    }
    edges->items = NULL;
    edges->count = 0;

    // For each node
    for (int i = 0; i < nodes->count; i++) {
        Node *node = &nodes->items[i];

        // Skip if no link
        if (node->link == NULL) {
            continue;
        }

        // Find the parent node by matching path with link
        Node *parent = NULL;
        for (int j = 0; j < nodes->count; j++) {
            if (strstr(nodes->items[j].path, node->link) != NULL) {
                parent = &nodes->items[j];
                break;
            }
        }

        if (parent == NULL) {
            continue;  // Parent not found
        }

        // Grow edges array
        Edge *tmp = realloc(edges->items, (edges->count + 1) * sizeof(Edge));
        if (tmp == NULL) {
            fprintf(stderr, "Failed to allocate memory for edges\n");
            exit(EXIT_FAILURE);
        }
        edges->items = tmp;

        // Create edge
        Edge *edge = &edges->items[edges->count];
        edge->src = node;
        edge->dst = parent;

        // Allocate and set label
        edge->label = malloc(strlen("parent") + 1);
        if (edge->label != NULL) {
            strcpy(edge->label, "parent");
        }

        edges->count++;
    }

    return edges;
}

/**
 * Print edges for debugging
 */
static void print_edges(EdgeList *edges) {
    printf("\n*** Edges: %d ***\n\n", edges->count);
    for (int i = 0; i < edges->count; i++) {
        const char *src_title = edges->items[i].src->title ?
                                edges->items[i].src->title :
                                edges->items[i].src->path;
        const char *dst_title = edges->items[i].dst->title ?
                                edges->items[i].dst->title :
                                edges->items[i].dst->path;
        const char *label = edges->items[i].label ? edges->items[i].label : "";

        printf("%d: [%s] --%s--> [%s]\n", i, src_title, label, dst_title);
    }
}

/**
 * Free edge list memory
 */
static void free_edges(EdgeList *edges) {
    if (edges == NULL) return;

    for (int i = 0; i < edges->count; i++) {
        free(edges->items[i].label);
    }
    free(edges->items);
    free(edges);
}

int tree(int argc, char **argv) {
    char *format = NULL;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--format") == 0) {
            if (i + 1 < argc) {
                format = argv[i + 1];
                i++;  // Skip next arg since we consumed it
            } else {
                fprintf(stderr, "Error: -f/--format requires a value\n");
                return 1;
            }
        }
    }

    // Compile regex patterns
    compile_regexes();

    // Process input from stdin and build node list
    NodeList *nodes = build_nodes();

    // Build edges from nodes
    EdgeList *edges = build_edges(nodes);

    // Print the node list
    print_nodes(nodes, format);

    // Print edges for debugging
    print_edges(edges);

    // Free memory
    free_edges(edges);
    free_nodes(nodes);
    free_regexes();

    return 0;
}