#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

int parse_args(
    int argc, char *argv[],
    char *scope_current, size_t scope_size, int scope_available_count, const char *scope_available[],
    char *method_current, size_t method_size, int method_available_count, const char *method_available[],
    char *pattern, size_t pattern_size
);
void print_help();
int check_terminal_stdin();
int file_matches_pattern(const char *filepath, regex_t *regex);



int main(int argc, char *argv[]) {
    // args
    char scope_current[64] = "all";
    const char *scope_available[] = {"all", "title", "summary", "content", "url"};
    int scope_available_count = sizeof(scope_available) / sizeof(scope_available[0]);

    char method_current[64] = "regex-simple";
    const char *method_available[] = {"regex-simple", "regex-bool"};
    int method_available_count = sizeof(method_available) / sizeof(method_available[0]);

    char pattern[256];
    char buffer[PATH_MAX];

    if (parse_args(argc, argv,
                   scope_current, sizeof(scope_current), scope_available_count, scope_available,
                   method_current, sizeof(method_current), method_available_count, method_available,
                   pattern, sizeof(pattern))) {
        print_help();
        return 1;
    }

    if (check_terminal_stdin()) {
        print_help();
        return 1;
    }

    if (strcmp(method_current, "regex-simple") == 0) {
        regex_t regex;

        if (regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB | REG_ICASE) != 0) {
            fprintf(stderr, "Could not compile regex\n");
            return 1;
        }

        while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            // Remove trailing newline
            buffer[strcspn(buffer, "\n")] = '\0';
            
            // Now we have a path in 'buffer'
            if (file_matches_pattern(buffer, &regex)) {
                printf("%s\n", buffer);  // Output matching path
            }
        }

        regfree(&regex);
        return 0;

    } else if (strcmp(method_current, "regex-bool") == 0) {
        // Use boolean regex (not implemented)
        fprintf(stderr, "Error: 'regex-bool' method not implemented\n");
        return 1;
    }

}

int file_matches_pattern(const char *filepath, regex_t *regex) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        return 0;  // Can't open? Not a match. Silent failure.
    }
    
    char line[4096];
    int match = 0;
    
    // Read file line by line
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (regexec(regex, line, 0, NULL, 0) == 0) {
            match = 1;
            break;  // Found it, stop reading
        }
    }
    
    fclose(fp);
    return match;
}

int check_terminal_stdin() {
    /*
    If stdin is a terminal:
    isatty(STDIN_FILENO)  → 1 (true)

    If stdin is a pipe/file:
    isatty(STDIN_FILENO)  → 0 (false)
    */
    if (isatty(STDIN_FILENO)) {
        fprintf(stderr, "Error: No input provided via stdin\n");
        return 1;
    }
    return 0;
}

int parse_args(
    int argc, char *argv[], 
    char *scope_current, size_t scope_size, int scope_available_count, const char *scope_available[], 
    char *method_current, size_t method_size, int method_available_count, const char *method_available[], 
    char *pattern, size_t pattern_size) {
    
    if (argc < 2) {
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        //printf("Argument %d: %s\n", i, argv[i]);

        if (strcmp(argv[i], "--scope") == 0 && i + 1 < argc) {
            char *value = argv[i + 1];
            int valid = 0;

            for (int j = 0; j < scope_available_count; j++) {
                if (strcmp(value, scope_available[j]) == 0) {
                    strncpy(scope_current, value, scope_size - 1);
                    scope_current[scope_size - 1] = '\0';
                    valid = 1;
                    break;
                }
            }

            if (!valid) {
                fprintf(stderr, "Error: Invalid value for --scope: %s\n", value);
                return 1;
            }
            i++;  // Skip the value argument
        } else if (strcmp(argv[i], "--method") == 0 && i + 1 < argc) {
            char *value = argv[i + 1];
            int valid = 0;

            for (int j = 0; j < method_available_count; j++) {
                if (strcmp(value, method_available[j]) == 0) {
                    strncpy(method_current, value, method_size - 1);
                    method_current[method_size - 1] = '\0';
                    valid = 1;
                    break;
                }
            }

            if (!valid) {
                fprintf(stderr, "Error: Invalid value for --method: %s\n", value);
                return 1;
            }
            i++;  // Skip the value argument
        } else {
            strncpy(pattern, argv[i], pattern_size - 1);
            pattern[pattern_size - 1] = '\0'; // Ensure null-termination
        }
    }

    printf("---\n");
    printf("Current scope: %s\n", scope_current[0] ? scope_current : "not set");
    printf("Current method: %s\n", method_current[0] ? method_current : "not set");
    printf("Pattern: %s\n", pattern[0] ? pattern : "not set");
    printf("---\n");

    return 0;
}

void print_help() {
    printf("Usage: jig-search-re <pattern>\n");
    printf("Reads file paths from stdin and outputs those whose contents match the given regex pattern.\n");
    printf("Example: find . -type f | jig-search-re 'search_pattern'\n");
}