#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include "find.h"

#define MAX_DEPTH 100

/**
 * Display help information for the 'find' command.
 */
static void help(void) {
    printf("Usage: jig find [OPTIONS] [PATH]\n");
    printf("       jig-find [OPTIONS] [PATH]\n");
    printf("\n");
    printf("Find files in the filesystem.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help     Display this help and exit\n");
    printf("\n");
    printf("Examples:\n");
    printf("  jig find ./notes              Find files in specified path\n");
    printf("  jig find                      Find files in current directory\n");
}

/**
 * Recursively process directory and print all file paths.
 * Returns 0 on success, 1 on error.
 */
static int process_directory(const char *path, int depth) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char fullpath[PATH_MAX];
    size_t path_len;
    int has_trailing_slash;
    int has_error = 0;

    // Prevent infinite recursion from symlink loops
    if (depth > MAX_DEPTH) {
        fprintf(stderr, "Maximum directory depth exceeded: %s\n", path);
        return 1;
    }

    dir = opendir(path);
    if (dir == NULL) {
        fprintf(stderr, "Cannot open directory: %s\n", path);
        return 1;
    }

    path_len = strlen(path);
    has_trailing_slash = (path_len > 0 && path[path_len - 1] == '/');

    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Skip hidden files
        if (entry->d_name[0] == '.') {
            continue;
        }

        // Build full path, avoiding double slashes
        int len;
        if (has_trailing_slash) {
            len = snprintf(fullpath, PATH_MAX, "%s%s", path, entry->d_name);
        } else {
            len = snprintf(fullpath, PATH_MAX, "%s/%s", path, entry->d_name);
        }

        if (len >= PATH_MAX) {
            fprintf(stderr, "Path too long: %s/%s\n", path, entry->d_name);
            continue;
        }

        // Use lstat to detect symlinks and avoid following them
        if (lstat(fullpath, &statbuf) == -1) {
            fprintf(stderr, "Cannot stat %s: %s\n", fullpath, strerror(errno));
            continue;
        }

        // Skip symbolic links to prevent loops
        if (S_ISLNK(statbuf.st_mode)) {
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            // Recursively process subdirectory
            if (process_directory(fullpath, depth + 1) != 0) {
                has_error = 1;
                // Continue processing other entries despite error
            }
        } else {
            // Print the file path
            printf("%s\n", fullpath);
        }
    }

    closedir(dir);
    return has_error;
}

int find(int argc, char **argv) {
    if (argc >= 2) {
        // Check for help flag
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            help();
            return 0;
        }

        // Process directory from argument
        return process_directory(argv[1], 0);
    }

    // No path provided - use current directory as default
    return process_directory(".", 0);
}
