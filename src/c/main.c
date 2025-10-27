# include <stdio.h>
# include <string.h>

int help();

int main(int argc, char *argv[]) {
    //printf("argc: %d\n", argc);
    //printf("argv[0]: %s\n", argv[0]);

    if (argc == 1) {
        help();
    }

    if (argc == 2) {
        if ((strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0)) {
            help();
        }
    }
    return 0;
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