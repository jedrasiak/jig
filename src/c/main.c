# include <stdio.h>
# include <string.h>
# include <sys/stat.h>

#ifdef _WIN32
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#endif

char *VERSION = "0.1.0";
char *LANGUAGES[] = {"en", "pl"};

int help();
int version();
int nn(char *path);

int main(int argc, char *argv[]) {
    //printf("argc: %d\n", argc);
    //printf("argv[0]: %s\n", argv[0]);

    if (argc == 1) {
        help();
    }

    if (argc == 2) {
        if ((strcmp(argv[1], "--version") == 0) || (strcmp(argv[1], "-v") == 0)) {
            version();
        } else {
            help();
        }
    }

    if (argc == 3) {
        if (strcmp(argv[1], "nn") == 0) {
            nn(argv[2]);
        } else {
            help();
        }
    }

    return 0;
}

int nn(char *path) {
    // Extract title (last component of path)
    char *title = path;
    char *last_slash = strrchr(path, '/');

    #ifdef _WIN32
        char *last_backslash = strrchr(path, '\\');
        // Use whichever separator is last
        if (last_backslash && (!last_slash || last_backslash > last_slash)) {
            last_slash = last_backslash;
        }
    #endif

    if (last_slash != NULL) {
        title = last_slash + 1;  // Skip the separator
    }

    // Create directory with rwxr-xr-x permissions (0755)
    if (mkdir(path, 0755) == 0) {

        for (int i = 0; i < sizeof(LANGUAGES) / sizeof(LANGUAGES[0]); i++) {
            char filepath[256];
            snprintf(filepath, sizeof(filepath), "%s/index.%s.md", path, LANGUAGES[i]);

            FILE *file = fopen(filepath, "w");
            if (file == NULL) {
                perror("Error creating file");
                return 1;
            }

            fprintf(file, "---\n");
            fprintf(file, "title: %s\n", title);
            fprintf(file, "slug: %s\n", title);
            fprintf(file, "---\n");

            fclose(file);
            printf("File created successfully: %s\n", filepath);
        }

        printf("Note created successfully: %s\n", path);
        return 0;
    } else {
        perror("Error creating directory");
        return 1;
    }

}

int help() {
    printf("Usage:\n");
    printf("  scientia <command> [options]\n");
    printf("\n");
    printf("Commands:\n");
    printf("  nn     Create a new note\n");
    printf("  ne     Create a new edge\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help       Show this help message and exit\n");
    printf("  -v, --version    Show version information\n");
    return 0;
}

int version() {
    printf("%s\n", VERSION);
    return 0;
}