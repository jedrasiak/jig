#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "files.h"

int search(const char *query, const char *path);

int search(const char *query, const char *path) {
    printf("Searching for: %s in %s\n", query, path);
    //int errors = traverse(path, 0, detail);

    markdown_files_count = 0;
    int errors = get_markdown_files(path, 0);

    if (errors > 0) {
        fprintf(stderr, "\nCompleted with %d errors\n", errors);
        return 1;
    } else {
        for (int i = 0; i < markdown_files_count; i++) {
            printf("%d: %s\n", i + 1, markdown_files_list[i]);
        }
    }

    return 0;
}


