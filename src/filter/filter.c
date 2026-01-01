#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <limits.h>
#include "filter.h"

/**
 * Display help message following Unix conventions
 */
static void help(void) {
    printf("Usage: jig filter [OPTIONS] [FILE]\n");
    printf("       jig-filter [OPTIONS] [FILE]\n");
    printf("       <command> | jig filter\n");
    printf("\n");
    printf("Filter note files based on YAML frontmatter criteria.\n");
    printf("\n");
    printf("Valid files must contain:\n");
    printf("  - YAML frontmatter delimited by --- at the beginning\n");
    printf("  - id property (maximum 36 bytes)\n");
    printf("  - title property\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help     Display this help and exit\n");
    printf("\n");
    printf("Examples:\n");
    printf("  jig filter note.md              Validate single file\n");
    printf("  find . -name \"*.md\" | jig filter  Filter multiple files\n");
}

/**
 * Check if file meets filtering criteria:
 * - Has YAML frontmatter (starts with ---)
 * - Has id property (max 36 bytes)
 * - Has title property
 * Returns 1 if valid, 0 otherwise
 */
static int validate_file(const char *filepath) {
    FILE *fptr;
    long filesize;
    char *filecontent;
    int is_valid = 0;
    regex_t rgx_id, rgx_title;
    int rgx_result;

    // Open file
    if ((fptr = fopen(filepath, "r")) == NULL) {
        return 0;
    }

    // Get file size
    fseek(fptr, 0L, SEEK_END);
    filesize = ftell(fptr);
    fseek(fptr, 0L, SEEK_SET);

    // Read file content
    filecontent = malloc(filesize + 1);
    if (filecontent == NULL) {
        fclose(fptr);
        return 0;
    }

    for (long j = 0; j < filesize; j++) {
        filecontent[j] = fgetc(fptr);
    }
    filecontent[filesize] = '\0';

    fclose(fptr);

    // Check for YAML frontmatter (must start with ---)
    if (strncmp(filecontent, "---", 3) != 0) {
        free(filecontent);
        return 0;
    }

    // Compile regex patterns
    if (regcomp(&rgx_id, "id:[ \t]*([^ \t\r\n]+)", REG_EXTENDED | REG_ICASE | REG_NEWLINE) != 0) {
        free(filecontent);
        return 0;
    }

    if (regcomp(&rgx_title, "title:[ \t]*([^ \t\r\n]+)", REG_EXTENDED | REG_ICASE | REG_NEWLINE) != 0) {
        regfree(&rgx_id);
        free(filecontent);
        return 0;
    }

    // Check for id property
    regmatch_t rgx_id_matches[2];
    rgx_result = regexec(&rgx_id, filecontent, 2, rgx_id_matches, 0);

    if (rgx_result == REG_NOMATCH) {
        regfree(&rgx_id);
        regfree(&rgx_title);
        free(filecontent);
        return 0;
    }

    // Validate id length (max 36 bytes)
    int id_length = rgx_id_matches[1].rm_eo - rgx_id_matches[1].rm_so;
    if (id_length > 36) {
        regfree(&rgx_id);
        regfree(&rgx_title);
        free(filecontent);
        return 0;
    }

    // Check for title property
    regmatch_t rgx_title_matches[2];
    rgx_result = regexec(&rgx_title, filecontent, 2, rgx_title_matches, 0);

    if (rgx_result == REG_NOMATCH) {
        regfree(&rgx_id);
        regfree(&rgx_title);
        free(filecontent);
        return 0;
    }

    // All criteria met
    is_valid = 1;

    // Cleanup
    regfree(&rgx_id);
    regfree(&rgx_title);
    free(filecontent);

    return is_valid;
}

int filter(int argc, char **argv) {
    char filepath[PATH_MAX];

    if (argc >= 2) {
        // Check for help flag
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            help();
            return 0;
        }

        // Process single filepath from argv[1]
        if (validate_file(argv[1])) {
            printf("%s\n", argv[1]);
        }
        return 0;
    }

    // Check if stdin is a pipe (not a terminal)
    if (!isatty(fileno(stdin))) {
        // Read filepaths from stdin, one per line
        while (fgets(filepath, sizeof(filepath), stdin) != NULL) {
            // Remove trailing newline
            filepath[strcspn(filepath, "\n")] = '\0';

            // Validate and output path
            if (validate_file(filepath)) {
                printf("%s\n", filepath);
            }
        }
        return 0;
    }

    // No input provided - show help to stderr and exit with error
    help();
    return 1;
}
