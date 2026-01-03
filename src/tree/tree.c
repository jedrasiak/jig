#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tree.h"
#include "nodes/nodes.h"
#include "edges/edges.h"

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
    printf("  -f, --format FORMAT Output format (md for markdown links)\n");
    printf("\n");
    printf("Examples:\n");
    printf("  jig find . -p \"\\.md$\" | jig filter | jig tree        Generate tree from markdown files\n");
    printf("  jig find . -p \"\\.md$\" | jig filter | jig tree -f md   Output with markdown links\n");
}

/**
 * Find children of a node using edges
 * Returns number of children found
 */
static int get_children(EdgeList *edges, Node *parent, Node **children, int max_children) {
    int count = 0;
    for (int i = 0; i < edges->count && count < max_children; i++) {
        if (edges->items[i].dst == parent) {
            children[count++] = edges->items[i].src;
        }
    }
    return count;
}

/**
 * Print a node and its children recursively as a tree
 */
static void print_tree_node(Node *node, EdgeList *edges, int depth, char *prefix, int is_last, const char *format) {
    // Print current node
    if (depth == 0) {
        // Root node - no prefix or connector
        if (format != NULL && strcmp(format, "md") == 0) {
            const char *title = node->title ? node->title : "title";
            printf("[%s](%s)\n", title, node->path);
        } else {
            printf("%s\n", node->title ? node->title : node->path);
        }
    } else {
        // Child node - print with tree connector
        const char *connector = is_last ? "└── " : "├── ";
        if (format != NULL && strcmp(format, "md") == 0) {
            const char *title = node->title ? node->title : "title";
            printf("%s%s[%s](%s)\n", prefix, connector, title, node->path);
        } else {
            printf("%s%s%s\n", prefix, connector, node->title ? node->title : node->path);
        }
    }

    // Find children using edges
    Node *children[1024];  // Max children per node
    int children_count = get_children(edges, node, children, 1024);

    // Recursively print each child
    for (int i = 0; i < children_count; i++) {
        int is_child_last = (i == children_count - 1);

        // Build prefix for child's descendants
        char new_prefix[1024];
        if (depth == 0) {
            // Root node: children get empty prefix
            new_prefix[0] = '\0';
        } else {
            // Non-root: add vertical bar or spaces
            snprintf(new_prefix, sizeof(new_prefix), "%s%s",
                     prefix,
                     is_last ? "    " : "│   ");
        }

        // Recursively print this child
        print_tree_node(children[i], edges, depth + 1, new_prefix, is_child_last, format);
    }
}

/**
 * Print tree structure starting from root nodes
 */
static void print_tree(NodeList *nodes, EdgeList *edges, const char *format) {
    // Find and print all root nodes (nodes with no parent link)
    for (int i = 0; i < nodes->count; i++) {
        if (nodes->items[i].link == NULL) {
            print_tree_node(&nodes->items[i], edges, 0, "", 0, format);
        }
    }
}

/**
 * Entry point for tree command
 */
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

    // Initialize node parser
    if (init_node_parser() != 0) {
        return 1;
    }

    // Build nodes from stdin
    NodeList *nodes = build_nodes_from_stdin();
    if (nodes == NULL) {
        cleanup_node_parser();
        return 1;
    }

    // Build edges from nodes
    EdgeList *edges = build_edges_from_nodes(nodes);
    if (edges == NULL) {
        free_nodes(nodes);
        cleanup_node_parser();
        return 1;
    }

    // Print tree structure
    print_tree(nodes, edges, format);

    // Cleanup
    free_edges(edges);
    free_nodes(nodes);
    cleanup_node_parser();

    return 0;
}
