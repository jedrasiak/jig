#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uuid.h"

/**
 * Display help message for the UUID CLI tool.
 */
static void help(void) {
    printf("Usage: jig-uuid [OPTIONS]\n");
    printf("\n");
    printf("Generate UUIDs (Universally Unique Identifiers).\n");
    printf("\n");
    printf("Currently supports only UUID version 7 (Unix Epoch time-based).\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help          Display this help and exit\n");
    printf("  -v <version>        UUID version to generate (default: 7)\n");
    printf("  -n <count>          Generate N UUIDs (default: 1)\n");
    printf("\n");
    printf("Output Format:\n");
    printf("  xxxxxxxx-xxxx-Vxxx-Nxxx-xxxxxxxxxxxx (36 characters)\n");
    printf("  Where V indicates version, N is variant bits (8, 9, a, or b)\n");
    printf("\n");
    printf("Examples:\n");
    printf("  jig-uuid                    Generate one UUID v7\n");
    printf("  jig-uuid -v 7 -n 10         Generate 10 UUIDs v7\n");
    printf("  jig-uuid > ids.txt          Save UUID to file\n");
}

/**
 * Standalone executable for UUID generation.
 * Build with: make (from src/uuid directory)
 */
int main(int argc, char **argv) {
    int count = 1;     // Default: generate 1 UUID
    int version = 7;   // Default: version 7

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        } else if (strcmp(argv[i], "-v") == 0) {
            // Next argument should be the version
            if (i + 1 < argc) {
                version = atoi(argv[i + 1]);
                i++;  // Skip next argument
            } else {
                fprintf(stderr, "Missing argument for -v option\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-n") == 0) {
            // Next argument should be the count
            if (i + 1 < argc) {
                count = atoi(argv[i + 1]);
                if (count <= 0) {
                    fprintf(stderr, "Invalid count: %s (must be positive integer)\n", argv[i + 1]);
                    return 1;
                }
                i++;  // Skip next argument
            } else {
                fprintf(stderr, "Missing argument for -n option\n");
                return 1;
            }
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            fprintf(stderr, "Use -h or --help for usage information\n");
            return 1;
        }
    }

    // Generate and output UUIDs
    for (int i = 0; i < count; i++) {
        char *id = uuid(version);
        if (id == NULL) {
            return 1;  // Error already reported to stderr
        }
        printf("%s\n", id);
        free(id);
    }

    return 0;
}
