#include <stdio.h>
#include <string.h>

#include "init.h"

#define CONFIG_FILE "./jig.conf"

static void help(void);

int init(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        }
    }

    FILE *fptr = fopen(CONFIG_FILE, "r");
    if (fptr != NULL) {
        fclose(fptr);
        fprintf(stderr, "Error: %s already exists.\n", CONFIG_FILE);
        return 1;
    }

    fptr = fopen(CONFIG_FILE, "w");
    if (fptr == NULL) {
        fprintf(stderr, "Error: Unable to create %s.\n", CONFIG_FILE);
        return 1;
    }

    fprintf(fptr, "[provider.mistral]\n");
    fprintf(fptr, "key =\n");
    fprintf(fptr, "endpoint = https://api.mistral.ai/v1\n");

    fclose(fptr);
    printf("Created %s\n", CONFIG_FILE);
    return 0;
}

/* --- */
static void help(void) {
    printf("Usage: jig init [OPTIONS]\n");
    printf("\n");
    printf("Initialize a new jig configuration file in the current directory.\n");
    printf("\n");
    printf("OPTIONS:\n");
    printf("  -h, --help    Display this help message\n");
    printf("\n");
}
