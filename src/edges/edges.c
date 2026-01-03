#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "edges.h"

/**
 * Display help message
 */
static void help(void) {
    printf("Usage: jig edges [OPTIONS]\n");
    printf("       jig-edges [OPTIONS]\n");
    printf("\n");
    printf("Extract and list edges (relationships) between nodes.\n");
    printf("\n");
    printf("Reads node CSV from stdin and outputs edge information as CSV.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help          Display this help and exit\n");
    printf("\n");
    printf("Output Format:\n");
    printf("  CSV with columns: src_id,src_title,dst_id,dst_title,label,src_path,dst_path\n");
    printf("\n");
    printf("Examples:\n");
    printf("  jig find . -p \"\\.md$\" | jig filter | jig nodes | jig edges\n");
    printf("  jig-find . | jig-filter | jig-nodes | jig-edges\n");
}

/**
 * Build edges from nodes by matching links to paths
 * Returns EdgeList* or NULL on error
 */
EdgeList* build_edges_from_nodes(NodeList *nodes) {
    EdgeList *edges = malloc(sizeof(EdgeList));
    if (edges == NULL) {
        fprintf(stderr, "Failed to allocate EdgeList\n");
        return NULL;
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
            free_edges(edges);
            return NULL;
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
 * Print edges as CSV to stdout
 * Format: src_id,src_title,dst_id,dst_title,label,src_path,dst_path
 */
void print_edges_csv(EdgeList *edges) {
    printf("src_id,src_title,dst_id,dst_title,label,src_path,dst_path\n");
    for (int i = 0; i < edges->count; i++) {
        Edge *edge = &edges->items[i];

        printf("%s,%s,%s,%s,%s,%s,%s\n",
               edge->src->id[0] ? edge->src->id : "",
               edge->src->title ? edge->src->title : "",
               edge->dst->id[0] ? edge->dst->id : "",
               edge->dst->title ? edge->dst->title : "",
               edge->label ? edge->label : "",
               edge->src->path ? edge->src->path : "",
               edge->dst->path ? edge->dst->path : "");
    }
}

/**
 * Free edge list memory
 */
void free_edges(EdgeList *edges) {
    if (edges == NULL) return;

    for (int i = 0; i < edges->count; i++) {
        free(edges->items[i].label);
    }
    free(edges->items);
    free(edges);
}

/**
 * Parse a CSV line into a Node
 * Expected format: id,title,path,link
 * Returns 0 on success, 1 on error
 */
static int parse_node_csv_line(Node *node, const char *line) {
    char *line_copy = strdup(line);
    if (line_copy == NULL) {
        return 1;
    }

    // Initialize node fields
    node->id[0] = '\0';
    node->title = NULL;
    node->path = NULL;
    node->link = NULL;

    // Parse CSV fields
    char *token;
    char *saveptr;
    int field = 0;

    token = strtok_r(line_copy, ",", &saveptr);
    while (token != NULL && field < 4) {
        switch (field) {
            case 0:  // id
                strncpy(node->id, token, 36);
                node->id[36] = '\0';
                break;
            case 1:  // title
                if (strlen(token) > 0) {
                    node->title = strdup(token);
                }
                break;
            case 2:  // path
                if (strlen(token) > 0) {
                    node->path = strdup(token);
                }
                break;
            case 3:  // link
                if (strlen(token) > 0) {
                    node->link = strdup(token);
                }
                break;
        }
        field++;
        token = strtok_r(NULL, ",", &saveptr);
    }

    free(line_copy);
    return 0;
}

/**
 * Read node CSV from stdin and build NodeList
 * Returns NodeList* or NULL on error
 */
static NodeList* read_nodes_from_csv(void) {
    char line[PATH_MAX];
    int is_header = 1;

    NodeList *list = malloc(sizeof(NodeList));
    if (list == NULL) {
        fprintf(stderr, "Failed to allocate NodeList\n");
        return NULL;
    }
    list->items = NULL;
    list->count = 0;

    while (fgets(line, sizeof(line), stdin) != NULL) {
        // Skip header line
        if (is_header) {
            is_header = 0;
            continue;
        }

        // Remove trailing newline
        line[strcspn(line, "\n")] = '\0';

        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }

        // Grow array
        Node *tmp = realloc(list->items, (list->count + 1) * sizeof(Node));
        if (tmp == NULL) {
            fprintf(stderr, "Failed to allocate memory for nodes\n");
            // Free existing nodes
            for (int i = 0; i < list->count; i++) {
                free(list->items[i].title);
                free(list->items[i].path);
                free(list->items[i].link);
            }
            free(list->items);
            free(list);
            return NULL;
        }
        list->items = tmp;

        // Parse node from CSV line
        Node *node = &list->items[list->count];
        if (parse_node_csv_line(node, line) != 0) {
            continue;  // Skip malformed lines
        }

        list->count++;
    }

    return list;
}

/**
 * Free nodes read from CSV
 */
static void free_csv_nodes(NodeList *list) {
    if (list == NULL) return;

    for (int i = 0; i < list->count; i++) {
        free(list->items[i].title);
        free(list->items[i].path);
        free(list->items[i].link);
    }
    free(list->items);
    free(list);
}

/**
 * Entry point for edges command
 * Expects node CSV on stdin, outputs edge CSV
 */
int edges(int argc, char **argv) {
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        }
    }

    // Read nodes from CSV stdin
    NodeList *nodes = read_nodes_from_csv();
    if (nodes == NULL) {
        return 1;
    }

    // Build edges from nodes
    EdgeList *edge_list = build_edges_from_nodes(nodes);
    if (edge_list == NULL) {
        free_csv_nodes(nodes);
        return 1;
    }

    // Print edges as CSV
    print_edges_csv(edge_list);

    // Cleanup
    free_edges(edge_list);
    free_csv_nodes(nodes);

    return 0;
}
