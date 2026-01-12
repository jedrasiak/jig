#include <stdio.h>
#include <string.h>

#include "ocr.h"
#include "config/config.h"

static void help(void);

int ocr(int argc, char **argv) {
    char arg_provider[MAX_NAME] = "mistral";  // Default provider
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

    if (strcmp(provider->name, "mistral") == 0) {
        printf("Using Mistral OCR provider\n");
        free_settings(&settings);
        return 0;
    } else {
        fprintf(stderr, "Error: Unsupported provider '%s'\n", provider->name);
        free_settings(&settings);
        return 1;
    }
}

/* --- */
static void help(void) {
    printf("Usage: jig ocr [OPTIONS]\n");
    printf("\n");
    printf("Perform OCR on the specified input.\n");
    printf("\n");
    printf("OPTIONS:\n");
    printf("  -p, --provider NAME  Specify OCR provider to use\n");
    printf("  -h, --help           Display this help message\n");
    printf("\n");
}