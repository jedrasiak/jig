#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "fn.h"

#define MAX_PATH_LENGTH 4096

int fn(int argc, char **argv) {
    printf(":jig:fn\n");
    char path[MAX_PATH_LENGTH];

    // Check if filepath provided as positional argument
    if (argc >= 2) {
        // Process single filepath from argv[1]
        printf("%s\n", argv[1]);
        return 0;
    }

    // Check if stdin is a pipe (not a terminal)
    if (!isatty(fileno(stdin))) {
        // Read filepaths from stdin, one per line
        while (fgets(path, sizeof(path), stdin) != NULL) {
            // Remove trailing newline
            size_t len = strlen(path);
            if (len > 0 && path[len - 1] == '\n') {
                path[len - 1] = '\0';
            }

            // Output the path
            printf("%s\n", path);
        }
        return 0;
    }

    // No input provided
    fprintf(stderr, "Error: No filepath provided. Use: jig fn <filepath> or pipe input.\n");
    return 1;
}
