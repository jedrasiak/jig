#include <stdio.h>
#include <string.h>

#include "files.h"

int search(const char *path, const char *algorithm, const char *output, const char *query);

int search(const char *path, const char *algorithm, const char *output, const char *query) {
    printf("path: %s\n", path);
    printf("algorithm: %s\n", algorithm);
    printf("output: %s\n", output);
    printf("query: %s\n", query);
    printf("---\n\n");

    // regex algorithm
    if (strcmp(algorithm, "re") == 0) {
        text_files_count = 0;
        int errors = get_text_files(path, 0);

        if (errors > 0) {
            fprintf(stderr, "\nCompleted with %d errors\n", errors);
            return 1;
        } else {
            for (int i = 0; i < text_files_count; i++) {
                //printf("%d: %s\n", i + 1, text_files_list[i]);
                char *path = text_files_list[i];
                printf("%s:\n", path);
            }
        }

        return 0;
    } else {
        fprintf(stderr, "Error: Unknown search algorithm '%s'\n", algorithm);
        return 1;
    }


}


