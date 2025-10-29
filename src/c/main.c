#include <stdio.h>
#include <string.h>
#include "vertex.h"

char *VERSION = "0.1.0";

int help();
int version();

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
        if (strcmp(argv[1], "nv") == 0) {
            nv(argv[2]);
        } else {
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
    printf("  nv <path>     Create a new vertex at the specified path\n");
    printf("  ne            Create a new edge\n");
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