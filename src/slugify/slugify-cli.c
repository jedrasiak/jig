#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "slugify.h"

/**
 * Display help message for the slugify CLI tool.
 */
static void help(void) {
    printf("Usage: jig-slugify [OPTIONS] TEXT\n");
    printf("\n");
    printf("Convert text to URL-friendly slugs.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help          Display this help and exit\n");
    printf("\n");
    printf("Examples:\n");
    printf("  jig-slugify \"Hello World\"        # Output: hello-world\n");
    printf("  jig-slugify \"My Blog Post 2024\"  # Output: my-blog-post-2024\n");
}

/**
 * Standalone executable for slug generation.
 * Build with: make (from src/slugify directory)
 */
int main(int argc, char **argv) {
    // Handle help flag
    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        help();
        return 0;
    }

    // Require exactly one argument
    if (argc != 2) {
        fprintf(stderr, "Error: exactly one TEXT argument required\n");
        fprintf(stderr, "Use -h or --help for usage information\n");
        return 1;
    }

    // Slugify and output
    char *slug = slugify(argv[1]);
    if (slug == NULL) {
        return 1;  // Error already reported to stderr
    }

    printf("%s\n", slug);
    free(slug);
    return 0;
}
