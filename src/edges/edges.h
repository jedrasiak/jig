#ifndef EDGES_H
#define EDGES_H

#include "nodes/nodes.h"

/**
 * Represents a directed edge (relationship) between two nodes
 *
 * Fields:
 *   src   - Pointer to source node
 *   dst   - Pointer to destination node
 *   label - Edge label (typically "parent")
 */
typedef struct {
    Node *src;
    Node *dst;
    char *label;
} Edge;

/**
 * Dynamic array of edges
 *
 * Fields:
 *   items - Array of Edge structures
 *   count - Number of edges in the array
 */
typedef struct {
    Edge *items;
    int count;
} EdgeList;

/**
 * Build edge list from node list by matching parent links
 * Creates directed edges from nodes to their parents
 * Returns EdgeList* or NULL on error
 */
EdgeList* build_edges_from_nodes(NodeList *nodes);

/**
 * Print edges as CSV to stdout
 * Format: src_id,src_title,dst_id,dst_title,label,src_path,dst_path
 */
void print_edges_csv(EdgeList *edges);

/**
 * Free edge list memory
 */
void free_edges(EdgeList *edges);

/**
 * Entry point for edges command
 * Reads node CSV from stdin, builds edges, outputs edge CSV
 * Returns 0 on success, non-zero on error
 */
int edges(int argc, char **argv);

#endif /* EDGES_H */