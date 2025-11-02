#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "files.h"
#include "re.h"

int search(const char *path, const char *algorithm, const char *format, const char *query);

int search(const char *path, const char *algorithm, const char *format, const char *query) {
    printf("path: %s\n", path);
    printf("algorithm: %s\n", algorithm);
    printf("format: %s\n", format);
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
                char *file_path = text_files_list[i];

                // open file
                FILE *file = fopen(file_path, "r");
                if (!file) {
                    fprintf(stderr, "Error: Could not open file '%s'\n", file_path);
                    continue;
                }

                // get file size
                fseek(file, 0, SEEK_END);
                long file_size = ftell(file);
                fseek(file, 0, SEEK_SET);

                // allocate buffer and read content
                char *content = malloc(file_size + 1);
                if (!content) {
                    fprintf(stderr, "Error: Could not allocate memory for '%s'\n", file_path);
                    fclose(file);
                    continue;
                }

                fread(content, 1, file_size, file);
                content[file_size] = '\0';
                fclose(file);

                // search for matches
                int count = re(query, content);

                // print result
                if (count > 0) {
                    printf("%s: %d\n", file_path, count);
                }

                // cleanup
                free(content);
            }
        }

        return 0;
    } else {
        fprintf(stderr, "Error: Unknown search algorithm '%s'\n", algorithm);
        return 1;
    }
}


