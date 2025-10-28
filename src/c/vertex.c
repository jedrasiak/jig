#include "vertex.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

// Supported languages for vertex creation
static char *LANGUAGES[] = {"en", "pl"};

int nv(char *path) {
    // Extract title (last component of path)
    char *title = path;
    char *last_slash = strrchr(path, '/');

    if (last_slash != NULL) {
        title = last_slash + 1;  // Skip the separator
    }

    // Create directory with rwxr-xr-x permissions (0755)
    if (mkdir(path, 0755) == 0) {

        for (size_t i = 0; i < sizeof(LANGUAGES) / sizeof(LANGUAGES[0]); i++) {
            char filepath[256];
            snprintf(filepath, sizeof(filepath), "%s/index.%s.md", path, LANGUAGES[i]);

            FILE *file = fopen(filepath, "w");
            if (file == NULL) {
                perror("Error creating file");
                return 1;
            }

            fprintf(file, "---\n");
            fprintf(file, "title: %s\n", title);
            fprintf(file, "slug: %s\n", title);
            fprintf(file, "---\n");

            fclose(file);
            printf("File created successfully: %s\n", filepath);
        }

        printf("Vertex created successfully: %s\n", path);
        return 0;
    } else {
        perror("Error creating directory");
        return 1;
    }
}
