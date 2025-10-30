#include "graph.h"
#include "cJSON.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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

int get_config(const char *graph_root, GraphConfig *config) {
    // Build path to graph.json
    char json_path[PATH_MAX];
    snprintf(json_path, sizeof(json_path), "%s/graph.json", graph_root);

    // Read file
    FILE *file = fopen(json_path, "r");
    if (!file) {
        return 0;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate buffer and read
    char *json_string = malloc(file_size + 1);
    if (!json_string) {
        fclose(file);
        return 0;
    }

    size_t read_size = fread(json_string, 1, file_size, file);
    json_string[read_size] = '\0';
    fclose(file);

    // Parse JSON
    cJSON *json = cJSON_Parse(json_string);
    free(json_string);

    if (!json) {
        return 0;
    }

    // Initialize config
    config->current_language = NULL;
    config->available_languages = NULL;
    config->num_languages = 0;

    // Extract languages object
    cJSON *languages = cJSON_GetObjectItem(json, "languages");
    if (languages) {
        // Get current language
        cJSON *current = cJSON_GetObjectItem(languages, "current");
        if (cJSON_IsString(current)) {
            config->current_language = strdup(current->valuestring);
        }

        // Get available languages array
        cJSON *available = cJSON_GetObjectItem(languages, "available");
        if (cJSON_IsArray(available)) {
            config->num_languages = cJSON_GetArraySize(available);
            config->available_languages = malloc(sizeof(char*) * config->num_languages);

            if (config->available_languages) {
                for (int i = 0; i < config->num_languages; i++) {
                    cJSON *lang = cJSON_GetArrayItem(available, i);
                    if (cJSON_IsString(lang)) {
                        config->available_languages[i] = strdup(lang->valuestring);
                    } else {
                        config->available_languages[i] = NULL;
                    }
                }
            }
        }
    }

    cJSON_Delete(json);
    return 1;
}

void free_config(GraphConfig *config) {
    if (config->current_language) {
        free(config->current_language);
        config->current_language = NULL;
    }

    if (config->available_languages) {
        for (int i = 0; i < config->num_languages; i++) {
            if (config->available_languages[i]) {
                free(config->available_languages[i]);
            }
        }
        free(config->available_languages);
        config->available_languages = NULL;
    }

    config->num_languages = 0;
}

// gcc -DTEST -I./include src/c/graph.c lib/cJSON.c -o bin/test_graph
#ifdef TEST

int main() {
    printf("=== Testing graph.c ===\n\n");

    // Test 1: is_root() with different paths
    printf("Test 1: is_root()\n");
    printf("  ✓ Current dir (.): %s\n", is_root(".") ? "IS root" : "NOT root");
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

        // Test 3: get_config() - parse graph.json
        printf("\nTest 3: get_config()\n");
        GraphConfig config;
        if (get_config(root, &config)) {
            printf("  ✓ Successfully parsed graph.json\n");
            printf("  Current language: %s\n", config.current_language ? config.current_language : "(none)");
            printf("  Available languages (%d):\n", config.num_languages);
            for (int i = 0; i < config.num_languages; i++) {
                printf("    - %s\n", config.available_languages[i] ? config.available_languages[i] : "(null)");
            }

            // Clean up
            free_config(&config);
            printf("  ✓ Config freed successfully\n");
        } else {
            printf("  ✗ Failed to parse graph.json\n");
        }
    } else {
        printf("  ✗ No graph root found\n");
    }

    return 0;
}
#endif
