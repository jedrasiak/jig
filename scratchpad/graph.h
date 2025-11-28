#ifndef GRAPH_H
#define GRAPH_H

#include <stddef.h>

// Graph configuration structure
typedef struct {
    char *current_language;      // Current language (e.g., "en")
    char **available_languages;  // Array of language codes
    int num_languages;           // Number of available languages
} GraphConfig;

// Check if directory contains graph.json
int is_root(const char *path);

// Find graph root by traversing upward, returns absolute path
int find_root(char *buffer, size_t size);

// Parse graph.json and populate config structure
// Returns 1 on success, 0 on failure
int get_config(const char *graph_root, GraphConfig *config);

// Free memory allocated in GraphConfig
void free_config(GraphConfig *config);

#endif