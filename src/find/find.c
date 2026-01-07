#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <regex.h>
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
    printf("  -h, --help             Display this help and exit\n");
    printf("  -p, --pattern PATTERN  Filter files by regex pattern (filename only)\n");
    printf("\n");
    printf("Examples:\n");
    printf("  jig find ./notes                Find files in specified path\n");
    printf("  jig find                        Find files in current directory\n");
    printf("  jig find -p '\\.md$' ./notes     Find markdown files only\n");
    printf("  jig find -p '^test'             Find files starting with 'test'\n");
}

/**
 * Recursively process directory and print all file paths.
 * If pattern is provided, only files matching the pattern are printed.
 * Returns 0 on success, 1 on error.
 */
static int process_directory(const char *path, int depth, regex_t *pattern) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char fullpath[PATH_MAX];
    size_t path_len;
    int has_trailing_slash;
    int has_error = 0;

    // Prevent infinite recursion
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
            if (process_directory(fullpath, depth + 1, pattern) != 0) {
                has_error = 1;
                // Continue processing other entries despite error
            }
        } else {
            // Extract filename from full path
            const char *filename = strrchr(fullpath, '/');
            if (filename != NULL) {
                filename++; // Skip the '/' character
            } else {
                filename = fullpath; // No '/' found, use entire path
            }

            // Check if filename matches pattern (if provided)
            int matches = 1; // Default to match if no pattern
            if (pattern != NULL) {
                int rgx_result = regexec(pattern, filename, 0, NULL, 0);
                matches = (rgx_result == 0);
            }

            // Print the file path if it matches
            if (matches) {
                printf("%s\n", fullpath);
            }
        }
    }

    closedir(dir);
    return has_error;
}

int find(int argc, char **argv) {
    char *pattern_str = NULL;
    char *path = NULL;
    regex_t rgx_pattern;
    regex_t *pattern_ptr = NULL;
    int rgx_result;
    int exit_status;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--pattern") == 0) {
            // Next argument should be the pattern
            if (i + 1 < argc) {
                pattern_str = argv[i + 1];
                i++; // Skip next argument
            } else {
                fprintf(stderr, "Error: -p/--pattern requires a pattern argument\n");
                return 1;
            }
        } else if (argv[i][0] != '-') {
            // Non-option argument is the path
            path = argv[i];
        } else {
            fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
            help();
            return 1;
        }
    }

    // Compile regex pattern if provided
    if (pattern_str != NULL) {
        rgx_result = regcomp(&rgx_pattern, pattern_str, REG_EXTENDED | REG_ICASE | REG_NEWLINE);
        if (rgx_result != 0) {
            char error_message[100];
            regerror(rgx_result, &rgx_pattern, error_message, sizeof(error_message));
            fprintf(stderr, "Could not compile regex pattern: %s\n", error_message);
            return 1;
        }
        pattern_ptr = &rgx_pattern;
    }

    // Use current directory if no path specified
    if (path == NULL) {
        path = ".";
    }

    // Process directory
    exit_status = process_directory(path, 0, pattern_ptr);

    // Free compiled regex pattern if it was used
    if (pattern_ptr != NULL) {
        regfree(&rgx_pattern);
    }

    return exit_status;
}
