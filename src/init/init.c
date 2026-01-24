#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "init.h"

#define CONFIG_FILE "./jig.conf"
#define CONFIG_ENTRY "jig.conf"
#define GITIGNORE_FILE "./.gitignore"
#define CLAUDE_DIR "./.claude"
#define CLAUDE_SETTINGS_FILE "./.claude/settings.local.json"

static void help(void);

int init(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        }
    }

    FILE *fptr;

    // Step 1: Create jig.conf
    fptr = fopen(CONFIG_FILE, "r");
    if (fptr != NULL) {
        fclose(fptr);
        printf("Skipped %s (already exists)\n", CONFIG_FILE);
    } else {
        fptr = fopen(CONFIG_FILE, "w");
        if (fptr == NULL) {
            fprintf(stderr, "Error: Unable to create %s.\n", CONFIG_FILE);
        } else {
            fprintf(fptr, "[provider.mistral]\n");
            fprintf(fptr, "key =\n");
            fprintf(fptr, "endpoint = https://api.mistral.ai/v1\n");
            fclose(fptr);
            printf("Created %s\n", CONFIG_FILE);
        }
    }

    // Step 2: Add jig.conf to .gitignore if it exists
    fptr = fopen(GITIGNORE_FILE, "r");
    if (fptr != NULL) {
        char line[256];
        int found = 0;
        int last_char = '\n';
        int ch;
        while (fgets(line, sizeof(line), fptr) != NULL) {
            line[strcspn(line, "\n")] = 0;
            if (strcmp(line, CONFIG_ENTRY) == 0) {
                found = 1;
                break;
            }
        }
        // Find last character of file
        fseek(fptr, -1, SEEK_END);
        ch = fgetc(fptr);
        if (ch != EOF) {
            last_char = ch;
        }
        fclose(fptr);

        if (!found) {
            fptr = fopen(GITIGNORE_FILE, "a");
            if (fptr != NULL) {
                if (last_char != '\n') {
                    fprintf(fptr, "\n");
                }
                fprintf(fptr, "%s\n", CONFIG_ENTRY);
                fclose(fptr);
                printf("Added %s to %s\n", CONFIG_ENTRY, GITIGNORE_FILE);
            }
        }
    }

    // Step 3: Create .claude directory
    if (mkdir(CLAUDE_DIR, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error: Unable to create %s directory.\n", CLAUDE_DIR);
    }

    // Step 4: Create .claude/settings.local.json
    fptr = fopen(CLAUDE_SETTINGS_FILE, "r");
    if (fptr != NULL) {
        fclose(fptr);
        printf("Skipped %s (already exists)\n", CLAUDE_SETTINGS_FILE);
    } else {
        fptr = fopen(CLAUDE_SETTINGS_FILE, "w");
        if (fptr == NULL) {
            fprintf(stderr, "Error: Unable to create %s.\n", CLAUDE_SETTINGS_FILE);
        } else {
            fprintf(fptr, "{\n");
            fprintf(fptr, "  \"permissions\": {\n");
            fprintf(fptr, "    \"allow\": [\n");
            fprintf(fptr, "      \"Bash(jig:*)\"\n");
            fprintf(fptr, "    ]\n");
            fprintf(fptr, "  }\n");
            fprintf(fptr, "}\n");
            fclose(fptr);
            printf("Created %s\n", CLAUDE_SETTINGS_FILE);
        }
    }

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
