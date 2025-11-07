#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "files.h"

char text_files_list[MAX_FILES][MAX_PATH];
int text_files_count = 0;

int get_text_files(const char *path, int depth) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char fullpath[MAX_PATH];
    int errors = 0;

    // Safety check for depth
    if (depth > MAX_DEPTH) {
        fprintf(stderr, "Max depth exceeded: %s\n", path);
        return 1;
    }

    // Safety check for file count
    if (text_files_count >= MAX_FILES) {
        fprintf(stderr, "Max file limit reached (%d files)\n", MAX_FILES);
        return 1;
    }

    // Open the current directory
    dir = opendir(path);
    if (dir == NULL) {
        fprintf(stderr, "Cannot open %s: %s\n", path, strerror(errno));
        return 1;
    }

    // First pass: collect .md files
    while ((entry = readdir(dir)) != NULL) {
        // skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // skip hidden files
        if (entry->d_name[0] == '.') {
            continue;
        }

        // Build full path
        int len = snprintf(fullpath, MAX_PATH, "%s/%s", path, entry->d_name);
        if (len >= MAX_PATH) {
            fprintf(stderr, "Path too long: %s/%s\n", path, entry->d_name);
            errors++;
            continue;
        }

        // Get information about this entry
        if (stat(fullpath, &statbuf) == -1) {
            fprintf(stderr, "Cannot stat %s: %s\n", fullpath, strerror(errno));
            errors++;
            continue;
        }

        // Only process regular files in this pass
        if (S_ISREG(statbuf.st_mode)) {
            // check if it's a .md, .adoc, or .txt file
            if (strstr(entry->d_name, ".md") != NULL ||
                strstr(entry->d_name, ".adoc") != NULL ||
                strstr(entry->d_name, ".txt") != NULL) {
                // Check if we have space in the list
                if (text_files_count < MAX_FILES) {
                    snprintf(text_files_list[text_files_count], MAX_PATH, "%s", fullpath);
                    text_files_count++;
                } else {
                    fprintf(stderr, "Max file limit reached (%d files)\n", MAX_FILES);
                    closedir(dir);
                    return errors + 1;
                }
            }
        }
    }

    // Rewind to read directory again for subdirectories
    rewinddir(dir);

    // Second pass: process directories recursively
    while ((entry = readdir(dir)) != NULL) {
        // skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // skip hidden files
        if (entry->d_name[0] == '.') {
            continue;
        }

        // Build full path
        int len = snprintf(fullpath, MAX_PATH, "%s/%s", path, entry->d_name);
        if (len >= MAX_PATH) {
            fprintf(stderr, "Path too long: %s/%s\n", path, entry->d_name);
            errors++;
            continue;
        }

        // Get information about this entry
        if (stat(fullpath, &statbuf) == -1) {
            fprintf(stderr, "Cannot stat %s: %s\n", fullpath, strerror(errno));
            errors++;
            continue;
        }

        // Only process directories in this pass
        if (S_ISDIR(statbuf.st_mode)) {
            // Recursively traverse subdirectory
            errors += get_text_files(fullpath, depth + 1);
        }
    }

    closedir(dir);
    return errors;
}

// gcc -DTEST -I./include -o bin/test_files src/c/files.c
#ifdef TEST
int main() {
    // Test the get_text_files function
    const char *path = ".";  // Current directory

    // Reset counter
    text_files_count = 0;

    // List all text files (.md, .adoc, .txt)
    int errors = get_text_files(path, 0);

    // Print results
    printf("Found %d text files:\n", text_files_count);
    for (int i = 0; i < text_files_count; i++) {
        printf("%d: %s\n", i + 1, text_files_list[i]);
    }

    if (errors > 0) {
        fprintf(stderr, "\nCompleted with %d errors\n", errors);
        return 1;
    }

    return 0;
}
#endif