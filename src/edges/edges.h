#ifndef EDGES_H
#define EDGES_H

#include "nodes/nodes.h"

typedef struct {
    Node *src;
    Node *dst;
    char *label;
} Edge;

typedef struct {
    Edge *items;
    int count;
} EdgeList;

/* Public API */
EdgeList* build_edges_from_nodes(NodeList *nodes);
void print_edges_csv(EdgeList *edges);
void free_edges(EdgeList *edges);
int edges(int argc, char **argv);

#endif /* EDGES_H */