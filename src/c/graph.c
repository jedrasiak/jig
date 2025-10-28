#include "graph.h"
#include <string.h>
#include <stdio.h>
#include <limits.h>

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
    ;
}

// gcc -DTEST -I./include src/c/graph.c -o bin/test_graph
#ifdef TEST

int main() {
    printf(">> testing graph.c\n");
    printf("is_root(\".\") = %d\n", is_root("."));
    printf("is_root(\"playground/\") = %d\n", is_root("playground/"));
    return 0;
}
#endif
