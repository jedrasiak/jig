#ifndef EDGE_H
#define EDGE_H

#include "node/node.h"

typedef struct {
    Node *src;
    Node *dst;
    char *label;
} Edge;

typedef struct {
    Edge *items;
    int count;
} EdgeList;

#endif /* EDGE_H */