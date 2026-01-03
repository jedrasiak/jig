#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

/*
gcc -Wall -Wextra -Werror src/jig-tree.c -o bin/jig-tree
find datasets/simple -type f -name "*.md" | jig-tree
*/

int main() {
    printf("tree.c\n");

    char filepath[PATH_MAX];
    int items = 0;
    char **filearr = NULL;

    while (fgets(filepath, sizeof(filepath), stdin) != NULL) {
        // Remove trailing newline if present
        filepath[strcspn(filepath, "\n")] = '\0';

        // Grow array
        char **tmp = realloc(filearr, (items + 1) * sizeof(char*));
        if (tmp == NULL) {
            // Free previously allocated memory
            for (int i = 0; i < items; i++) {
                free(filearr[i]);
            }
            free(filearr);
            fprintf(stderr, "Realloc failed\n");
            exit(EXIT_FAILURE);
        }
        filearr = tmp;

        // Allocate and copy the filepath
        filearr[items] = malloc(strlen(filepath) + 1);
        if (filearr[items] == NULL) {
            for (int i = 0; i < items; i++) {
                free(filearr[i]);
            }
            free(filearr);
            fprintf(stderr, "Malloc failed\n");
            exit(EXIT_FAILURE);
        }
        strcpy(filearr[items], filepath);

        items++;
    }

    for (int i = 0; i < items; i++) {
        printf("%p | %s\n", (void*)&filearr[i], filearr[i]);

        FILE *fptr;
        // open file
        if ((fptr = fopen(filearr[i], "r")) == NULL) {
            fprintf(stderr, "File open failed");
            exit(EXIT_FAILURE);
        }

        // read file
        /*
        int ch;
        while((ch = getc(fptr)) != EOF) {
            putc(ch,stdout);
        }
        */
        fseek(fptr, 0L, SEEK_END);
        long fend = ftell(fptr);
        //printf("%ld\n", fend);
        fseek(fptr, 0L, SEEK_SET);

        char *fcont = malloc(fend + 1);

        for (int i = 0; i < fend; i++) {
            fcont[i] = getc(fptr);
        }
        fcont[fend] = '\0';

        printf("%s\n", fcont);
        free(fcont);

        // close file
        if (fclose(fptr) != 0) {
            fprintf(stderr, "File close failed");
            exit(EXIT_FAILURE);
        }
    }

    // Free all allocated memory
    for (int i = 0; i < items; i++) {
        free(filearr[i]);
    }
    free(filearr);

    return 0;
}
