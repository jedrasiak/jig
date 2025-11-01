#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_DEPTH 100
#define MAX_PATH 4096

// Callback function type for file processing
typedef void (*file_callback)(const char *fullpath, const struct stat *statbuf, int depth);

int traverse(const char *path, int depth, file_callback callback);
int search(const char *query, const char *path);
void detail(const char *fullpath, const struct stat *statbuf, int depth);

void detail(const char *fullpath, const struct stat *statbuf, int depth) {
    // Print indentation
    for (int i = 0; i < depth; i++) {
        printf("│   ");
    }

    // Print file name
    const char *filename = strrchr(fullpath, '/');
    filename = filename ? filename + 1 : fullpath;
    printf("├── %s", filename);

    // Print size in a human-readable way
    off_t size = statbuf->st_size;
    if (size < 1024) {
        printf(" [%ld B]", (long)size);
    } else if (size < 1024 * 1024) {
        printf(" [%.1f KB]", size / 1024.0);
    } else {
        printf(" [%.1f MB]", size / (1024.0 * 1024.0));
    }

    // Print permissions in rwx format
    printf(" [");
    printf((statbuf->st_mode & S_IRUSR) ? "r" : "-");
    printf((statbuf->st_mode & S_IWUSR) ? "w" : "-");
    printf((statbuf->st_mode & S_IXUSR) ? "x" : "-");
    printf((statbuf->st_mode & S_IRGRP) ? "r" : "-");
    printf((statbuf->st_mode & S_IWGRP) ? "w" : "-");
    printf((statbuf->st_mode & S_IXGRP) ? "x" : "-");
    printf((statbuf->st_mode & S_IROTH) ? "r" : "-");
    printf((statbuf->st_mode & S_IWOTH) ? "w" : "-");
    printf((statbuf->st_mode & S_IXOTH) ? "x" : "-");
    printf("]\n");
}

int search(const char *query, const char *path) {
    printf("Searching for: %s in %s\n", query, path);
    int errors = traverse(path, 0, detail);

    if (errors > 0) {
        fprintf(stderr, "\nCompleted with %d errors\n", errors);
        return 1;
    }

    return 0;
}

int traverse(const char *path, int depth, file_callback callback) {
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
                // Invoke callback if provided
                if (callback != NULL) {
                    callback(fullpath, &statbuf, depth);
                }
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
            errors += traverse(fullpath, depth + 1, callback);
        }
    }

    // Always close what you open, Dave
    closedir(dir);

    return errors;

}
