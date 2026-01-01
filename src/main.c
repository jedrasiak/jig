#include <stdio.h>
#include <string.h>

#include "filter/filter.h"
#include "find/find.h"
#include "tree/tree.h"

int main(int argc, char **argv) {
    
    /* DEBUG
    printf(":jig\n");
    printf("> argc: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("> argv[%d]: %s\n", i, argv[i]);
    }
    printf("---\n");
    */

    if (strcmp(argv[1], "filter") == 0) {
        return filter(argc - 1, argv + 1);
    }
    
    if (strcmp(argv[1], "find") == 0) {
        return find(argc - 1, argv + 1);
    }
    
    if (strcmp(argv[1], "tree") == 0) {
        return tree(argc - 1, argv + 1);
    }
    
    return 0;
}