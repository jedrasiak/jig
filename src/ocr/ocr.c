#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#include "ocr.h"
#include "config/config.h"

static void help(void);

int ocr(int argc, char **argv) {
    char arg_provider[MAX_NAME] = "mistral";  // Default provider
    char filepath[PATH_MAX] = "";
    FILE *file = NULL;
    char *extensions[] = {"pdf"};
    Provider *provider = NULL;

    // Load config first
    Settings settings = load_config();
    if (settings.providers.items == NULL) {
        return 1;
    }

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            free_settings(&settings);
            return 0;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--provider") == 0) {
            if (i + 1 < argc) {
                strncpy(arg_provider, argv[i + 1], MAX_NAME - 1);
                i++;  // Skip next arg since we consumed it
            } else {
                fprintf(stderr, "Error: -p/--provider requires a value\n");
                free_settings(&settings);
                return 1;
            }
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0) {
            if (i + 1 < argc) {
                strncpy(filepath, argv[i + 1], PATH_MAX - 1);
                i++;  // Skip next arg since we consumed it
            } else {
                fprintf(stderr, "Error: -f/--file requires a value\n");
                free_settings(&settings);
                return 1;
            }
        } else {
            fprintf(stderr, "Error: Unknown argument: %s\n", argv[i]);
            free_settings(&settings);
            return 1;
        }
    }

    // Check if provider is available in config
    provider = find_provider(&settings, arg_provider);
    if (provider == NULL) {
        fprintf(stderr, "Error: Provider '%s' not found in config\n", arg_provider);
        free_settings(&settings);
        return 1;
    }

    // Check filepath provided
    if (strlen(filepath) == 0) {
        fprintf(stderr, "Error: No input file specified.\n");
        help();
        free_settings(&settings);
        return 1;
    }

    // Open file
    file = fopen(filepath, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filepath);
        free_settings(&settings);
        return 1;
    }

    // check extension
    int extension_allowed = 0;
    const char *ext = strrchr(filepath, '.');

    for (size_t i = 0; i < sizeof(extensions) / sizeof(extensions[0]); i++) {
        if (ext != NULL && strcmp(ext + 1, extensions[i]) == 0) {
            extension_allowed = 1;
            break;
        }
    }

    if (!extension_allowed) {
        fprintf(stderr, "Error: Unsupported file extension.\n");
        help();
        free_settings(&settings);
        fclose(file);
        return 1;
    }

    if (strcmp(provider->name, "mistral") == 0) {
        // processing
        printf("Using Mistral OCR provider\n");

        // cleanup
        printf("OCR processing completed for file: %s\n", filepath);
        free_settings(&settings);
        fclose(file);
        return 0;
    } else {
        fprintf(stderr, "Error: Unsupported provider '%s'\n", provider->name);
        help();
        free_settings(&settings);
        fclose(file);
        return 1;
    }
}

/* --- */
static void help(void) {
    printf("Usage: jig ocr -f FILE [-p NAME] [-h]\n");
    printf("\n");
    printf("Perform OCR on the specified input.\n");
    printf("\n");
    printf("OPTIONS:\n");
    printf("  -f, --file FILE      Specify the file to process (required)\n");
    printf("  -p, --provider NAME  Specify OCR provider to use (default: mistral)\n");
    printf("  -h, --help           Display this help message\n");
    printf("\n");
    printf("SUPPORTED FILES:\n");
    printf("  pdf\n");
    printf("\n");
    printf("SUPPORTED PROVIDERS:\n");
    printf("  mistral\n");
}