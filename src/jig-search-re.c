#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>

int file_matches_pattern(const char *filepath, regex_t *regex);

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <pattern>\n", argv[0]);
        return 1;
    }

    // Check if stdin is a terminal (not a pipe)
    if (isatty(STDIN_FILENO)) {
        fprintf(stderr, "Error: This program expects input from a pipe\n");
        fprintf(stderr, "Usage: command | %s <pattern>\n", argv[0]);
        fprintf(stderr, "Example: find . -type f | %s <pattern>\n", argv[0]);
        return 1;
    }
    
    char buffer[PATH_MAX];
    char *pattern = argv[1];
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