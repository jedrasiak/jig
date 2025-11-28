#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "files.h"

char **text_files_list = NULL;
int text_files_count = 0;
int text_files_capacity = 0;

// First pass: count text files
int count_text_files(const char *path, int depth) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char fullpath[MAX_PATH];
    int count = 0;

    // Safety check for depth
    if (depth > MAX_DEPTH) {
        fprintf(stderr, "Max depth exceeded: %s\n", path);
        return 0;
    }

    // Open the current directory
    dir = opendir(path);
    if (dir == NULL) {
        fprintf(stderr, "Cannot open %s: %s\n", path, strerror(errno));
        return 0;
    }

    // Count files and recurse into directories
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
            continue;
        }

        // Get information about this entry
        if (stat(fullpath, &statbuf) == -1) {
            fprintf(stderr, "Cannot stat %s: %s\n", fullpath, strerror(errno));
            continue;
        }

        // Count regular text files
        if (S_ISREG(statbuf.st_mode)) {
            if (strstr(entry->d_name, ".md") != NULL ||
                strstr(entry->d_name, ".adoc") != NULL ||
                strstr(entry->d_name, ".txt") != NULL) {
                count++;
            }
        }
        // Recurse into subdirectories
        else if (S_ISDIR(statbuf.st_mode)) {
            count += count_text_files(fullpath, depth + 1);
        }
    }

    closedir(dir);
    return count;
}

// Initialize the text files list with the given capacity
void init_text_files_list(int count) {
    text_files_capacity = count;
    text_files_count = 0;

    if (count == 0) {
        text_files_list = NULL;
        return;
    }

    // Allocate array of string pointers
    text_files_list = (char **)malloc(count * sizeof(char *));
    if (text_files_list == NULL) {
        fprintf(stderr, "Failed to allocate memory for %d files\n", count);
        exit(1);
    }

    // Allocate each string
    for (int i = 0; i < count; i++) {
        text_files_list[i] = (char *)malloc(MAX_PATH * sizeof(char));
        if (text_files_list[i] == NULL) {
            fprintf(stderr, "Failed to allocate memory for file path %d\n", i);
            // Free previously allocated memory
            for (int j = 0; j < i; j++) {
                free(text_files_list[j]);
            }
            free(text_files_list);
            exit(1);
        }
    }
}

// Free the text files list
void free_text_files_list() {
    if (text_files_list == NULL) {
        return;
    }

    for (int i = 0; i < text_files_capacity; i++) {
        free(text_files_list[i]);
    }
    free(text_files_list);

    text_files_list = NULL;
    text_files_count = 0;
    text_files_capacity = 0;
}

// Second pass: populate the text files list
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

    // Safety check: ensure array was initialized
    if (text_files_list == NULL && text_files_capacity > 0) {
        fprintf(stderr, "Error: text_files_list not initialized\n");
        return 1;
    }

    // Open the current directory
    dir = opendir(path);
    if (dir == NULL) {
        fprintf(stderr, "Cannot open %s: %s\n", path, strerror(errno));
        return 1;
    }

    // First pass: collect text files
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
                // Store the file (should have space since we pre-counted)
                if (text_files_count < text_files_capacity) {
                    snprintf(text_files_list[text_files_count], MAX_PATH, "%s", fullpath);
                    text_files_count++;
                } else {
                    fprintf(stderr, "Warning: Found more files than expected\n");
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

// gcc -DTEST -I./include -o bin/test_files src/files.c
#ifdef TEST
int main() {
    // Test the get_text_files function
    const char *path = ".";  // Current directory

    // First pass: count files
    printf("Counting text files...\n");
    int count = count_text_files(path, 0);
    printf("Found %d text files\n\n", count);

    if (count == 0) {
        printf("No text files found\n");
        return 0;
    }

    // Allocate array
    init_text_files_list(count);

    // Second pass: populate list
    printf("Collecting file paths...\n");
    int errors = get_text_files(path, 0);

    // Print results
    printf("\nText files list:\n");
    for (int i = 0; i < text_files_count; i++) {
        printf("%d: %s\n", i + 1, text_files_list[i]);
    }

    printf("\nTotal: %d files\n", text_files_count);

    // Clean up
    free_text_files_list();

    if (errors > 0) {
        fprintf(stderr, "\nCompleted with %d errors\n", errors);
        return 1;
    }

    return 0;
}
#endif