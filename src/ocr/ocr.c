#include <stdio.h>
#include <string.h>

#include "ocr.h"

static void help(void);

int ocr(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        }
    }

    // Placeholder for OCR functionality
    printf("OCR functionality is not yet implemented.\n");
    return 0;
}

/* --- */
static void help(void) {
    printf("Usage: jig ocr [OPTIONS]\n");
    printf("\n");
    printf("Perform OCR on the specified input.\n");
    printf("\n");
    printf("OPTIONS:\n");
    printf("  -h, --help    Display this help message\n");
    printf("\n");
}