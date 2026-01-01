#include <stdio.h>
#include <string.h>
#include "fn/fn.h"

int main(int argc, char **argv) {
    printf(":jig\n");
    printf("> argc: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("> argv[%d]: %s\n", i, argv[i]);
    }
    printf("---\n");

    if (strcmp(argv[1], "fn") == 0) {
        return fn(argc - 1, argv + 1);
    }

    return 0;
}