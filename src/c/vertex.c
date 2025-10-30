#include "vertex.h"
#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#include <libgen.h>

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

    // Handle current directory case
    char vertex_path[PATH_MAX];
    char title_buffer[PATH_MAX];
    char *title;
    int create_dir = 1;

    if (vertex_name == NULL) {
        // Use current directory
        strcpy(vertex_path, ".");
        create_dir = 0;

        // Get current directory name for title
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("Error getting current directory");
            free_config(&config);
            return 1;
        }

        char *cwd_copy = strdup(cwd);
        char *dir_name = basename(cwd_copy);
        strncpy(title_buffer, dir_name, sizeof(title_buffer) - 1);
        title_buffer[sizeof(title_buffer) - 1] = '\0';
        title = title_buffer;
        free(cwd_copy);
        
        // Check if vertex.json exists
        if (access("vertex.json", F_OK) == 0) {
            fprintf(stderr, "Error: vertex.json already exists. Refusing to overwrite.\n");
            free_config(&config);
            return 1;
        }

        // Check if files already exist in current directory
        for (int i = 0; i < config.num_languages; i++) {
            char filepath[PATH_MAX];
            snprintf(filepath, sizeof(filepath), "index.%s.md",
                     config.available_languages[i]);
            if (access(filepath, F_OK) == 0) {
                fprintf(stderr, "Error: %s already exists. Refusing to overwrite.\n", filepath);
                free_config(&config);
                return 1;
            }
        }
    } else {
        // Use provided path
        strcpy(vertex_path, vertex_name);

        // Extract title (last component of path)
        title = vertex_name;
        char *last_slash = strrchr(vertex_name, '/');
        if (last_slash != NULL) {
            title = last_slash + 1;
        }
    }

    // Create directory if needed
    if (create_dir) {
        // Vertex is created relative to current working directory
        if (mkdir(vertex_path, 0755) != 0) {
            perror("Error creating directory");
            free_config(&config);
            return 1;
        }
    }

    // Create markdown files for each configured language
    for (int i = 0; i < config.num_languages; i++) {
        if (config.available_languages[i] == NULL) {
            continue;
        }

        char filepath[PATH_MAX];
        int len = snprintf(filepath, sizeof(filepath), "%s/index.%s.md",
                          vertex_path, config.available_languages[i]);

        if (len >= (int)sizeof(filepath)) {
            fprintf(stderr, "Error: Path too long for %s/index.%s.md\n",
                    vertex_path, config.available_languages[i]);
            free_config(&config);
            return 1;
        }

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

    // Create vertex.json file
    char json_filepath[PATH_MAX];
    int len = snprintf(json_filepath, sizeof(json_filepath), "%s/vertex.json", vertex_path);

    if (len >= (int)sizeof(json_filepath)) {
        fprintf(stderr, "Error: Path too long for vertex.json\n");
        free_config(&config);
        return 1;
    }

    FILE *json_file = fopen(json_filepath, "w");
    if (json_file == NULL) {
        perror("Error creating vertex.json");
        free_config(&config);
        return 1;
    }

    fprintf(json_file, "{}\n");
    fclose(json_file);
    printf("Created: %s\n", json_filepath);

    printf("✓ Vertex created: %s\n", create_dir ? vertex_path : "current directory");
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
