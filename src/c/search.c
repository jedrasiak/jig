#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_DEPTH 100
#define MAX_PATH 4096

int traverse(const char *path, int depth);
int search(const char *query, const char *path);

int search(const char *query, const char *path) {
    printf("Searching for: %s in %s\n", query, path);
    int errors = traverse(path, 0);

    if (errors > 0) {
        fprintf(stderr, "\nCompleted with %d errors\n", errors);
        return 1;
    }

    return 0;
}

int traverse(const char *path, int depth) {
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

    // Open the current directory
    dir = opendir(path);

    if (dir == NULL) {
        fprintf(stderr, "Cannot open %s: %s\n", path, strerror(errno));
        return 1;
    }

    // First pass: collect and print files
    while ((entry = readdir(dir)) != NULL) {

        // skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // skip hidden files
        if (entry->d_name[0] == '.') {
            continue;
        }

        // Build full path: path + "/" + filename
        // Check if path would overflow
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
            // skip not .md files
            if (strstr(entry->d_name, ".md") != NULL) {
                // Print with indentation
                for (int i = 0; i < depth; i++) {
                    printf("│   ");
                }
                printf("├── %s\n", entry->d_name);
            }
        }
    }

    // Rewind to read directory again for subdirectories
    rewinddir(dir);

    // Second pass: process directories
    while ((entry = readdir(dir)) != NULL) {

        // skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // skip hidden files
        if (entry->d_name[0] == '.') {
            continue;
        }

        // Build full path: path + "/" + filename
        // Check if path would overflow
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
            // Print with indentation
            for (int i = 0; i < depth; i++) {
                printf("│   ");
            }
            printf("├── %s/\n", entry->d_name);

            // RECURSION HAPPENS HERE
            errors += traverse(fullpath, depth + 1);
        }
    }

    // Always close what you open, Dave
    closedir(dir);

    return errors;

}