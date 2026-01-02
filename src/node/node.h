#ifndef NODE_H
#define NODE_H

typedef struct {
    char id[37];
    char *path;
    char *link;
    char *title;
} Node;

typedef struct {
    Node *items;
    int count;
} NodeList;

#endif /* NODE_H */