#include "vertex.h"
#include "graph.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>

int nv(char *vertex_name) {
    // Find graph root
    char graph_root[PATH_MAX];
    if (!find_root(graph_root, sizeof(graph_root))) {
        fprintf(stderr, "Error: Not inside a scientia graph.\n");
        fprintf(stderr, "Run 'scientia init' to create a new graph.\n");
        return 1;
    }

    // Load graph configuration
    GraphConfig config;
    if (!get_config(graph_root, &config)) {
        fprintf(stderr, "Error: Failed to parse graph.json\n");
        return 1;
    }

    // Check if we have any languages configured
    if (config.num_languages == 0) {
        fprintf(stderr, "Error: No languages configured in graph.json\n");
        free_config(&config);
        return 1;
    }

    // Extract title (last component of path)
    char *title = vertex_name;
    char *last_slash = strrchr(vertex_name, '/');
    if (last_slash != NULL) {
        title = last_slash + 1;
    }

    // Create directory with rwxr-xr-x permissions (0755)
    // Vertex is created relative to current working directory
    if (mkdir(vertex_name, 0755) != 0) {
        perror("Error creating directory");
        free_config(&config);
        return 1;
    }

    // Create markdown files for each configured language
    for (int i = 0; i < config.num_languages; i++) {
        if (config.available_languages[i] == NULL) {
            continue;
        }

        char filepath[PATH_MAX];
        snprintf(filepath, sizeof(filepath), "%s/index.%s.md",
                 vertex_name, config.available_languages[i]);

        FILE *file = fopen(filepath, "w");
        if (file == NULL) {
            perror("Error creating file");
            free_config(&config);
            return 1;
        }

        fprintf(file, "---\n");
        fprintf(file, "title: %s\n", title);
        fprintf(file, "slug: %s\n", title);
        fprintf(file, "---\n");

        fclose(file);
        printf("Created: %s\n", filepath);
    }

    printf("✓ Vertex created: %s\n", vertex_name);
    printf("  Languages: ");
    for (int i = 0; i < config.num_languages; i++) {
        printf("%s", config.available_languages[i]);
        if (i < config.num_languages - 1) {
            printf(", ");
        }
    }
    printf("\n");

    free_config(&config);
    return 0;
}
