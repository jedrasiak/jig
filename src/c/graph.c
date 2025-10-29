#include "graph.h"
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <libgen.h>

int is_root(const char *path) {
      char graph_file[PATH_MAX];
      snprintf(graph_file, sizeof(graph_file), "%s/graph.json", path);

      FILE *file = fopen(graph_file, "r");
      if (file) {
          fclose(file);
          return 1;
      }
      return 0;
}

int find_root(char *buffer, size_t size) {
    char current[PATH_MAX];
    char previous[PATH_MAX] = "";

    // Get current working directory (absolute path)
    if (getcwd(current, sizeof(current)) == NULL) {
        return 0;
    }

    while (1) {
        // Check if graph.json exists in current directory
        if (is_root(current)) {
            strncpy(buffer, current, size - 1);
            buffer[size - 1] = '\0';
            return 1;
        }

        // Check if we've reached the top (no more parents)
        if (strcmp(current, previous) == 0) {
            return 0; // Not found
        }

        // Save current path before moving up
        strncpy(previous, current, sizeof(previous) - 1);
        previous[sizeof(previous) - 1] = '\0';

        // Move up one directory
        char *parent = dirname(current);
        strncpy(current, parent, sizeof(current) - 1);
        current[sizeof(current) - 1] = '\0';
    }
}

// gcc -DTEST -I./include src/c/graph.c -o bin/test_graph
#ifdef TEST

int main() {
    printf("=== Testing graph.c ===\n\n");

    // Test 1: is_root() with different paths
    printf("Test 1: is_root()\n");
    printf("  Current dir (.): %s\n", is_root(".") ? "IS root" : "NOT root");
    printf("  Playground: %s\n", is_root("playground/") ? "IS root" : "NOT root");
    printf("\n");

    // Test 2: find_root() - traverse upward
    printf("Test 2: find_root()\n");
    char root[PATH_MAX];
    if (find_root(root, sizeof(root))) {
        printf("  ✓ Found graph root: %s\n", root);

        // Verify by checking if graph.json exists there
        if (is_root(root)) {
            printf("  ✓ Verified: graph.json exists at root\n");
        }
    } else {
        printf("  ✗ No graph root found\n");
    }

    return 0;
}
#endif
