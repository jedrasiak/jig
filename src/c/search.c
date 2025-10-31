#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

int search(const char *phrase) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char fullpath[1024];
    
    // Open the current directory
    dir = opendir(".");
    
    if (dir == NULL) {
        printf("Failed to open directory\n");
        return 1;
    }
    
    printf("Searching for: %s\n", phrase);
    printf("%-20s %8s %12s %12s %12s %12s\n", "Name", "Type", "Inode", "st_mode", "type", "path");
    printf("%-20s %8s %12s %12s %12s %12s\n", "----", "----", "-----", "-------", "----", "----");

    // Read entries one by one
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
        snprintf(fullpath, sizeof(fullpath), "%s/%s", ".", entry->d_name);

        // Get information about this entry
        if (stat(entry->d_name, &statbuf) == -1) {
            printf("Failed to stat %s\n", entry->d_name);
            continue;
        }
        
        // get type as string
        char type_str[4];

        if (S_ISDIR(statbuf.st_mode)) {
            entry->d_type = DT_DIR;
            strcpy(type_str, "DIR");
        } else if (S_ISREG(statbuf.st_mode)) {
            entry->d_type = DT_REG;
            strcpy(type_str, "REG");
        } else {
            entry->d_type = DT_UNKNOWN;
        }

        printf("%-20s %8d %12ld %12o %12s %12s\n", entry->d_name, entry->d_type, entry->d_ino, statbuf.st_mode, type_str, fullpath);
    }
    
    // Always close what you open, Dave
    closedir(dir);

    return 0;
}