#include <stdio.h>
#include <string.h>
#include <regex.h>

#include "re.h"

int re(const char *pattern, const char *string) {
    // declare regex object
    regex_t regex;
    regmatch_t match;
    const char *cursor = string;
    int count = 0;
    
    // compile the regex pattern
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        fprintf(stderr, "Could not compile pattern: %s\n", pattern);
        return -1;
    }
    
    // find matches
    while (regexec(&regex, cursor, 1, &match, 0) == 0) {
        count++;
        cursor += (match.rm_eo > 0) ? match.rm_eo : 1;
        if (*cursor == '\0') break;
    }

    // cleanup
    regfree(&regex);
    return count;
}

// gcc -DTEST -o bin/test_re src/c/re.c -Wall -Wextra -pedantic
#ifdef TEST
int main(int argc, char *argv[]) {
    int count = 0;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <pattern> <string>\n", argv[0]);
        return 1;
    } else {
        const char *pattern = argv[1];
        const char *string = argv[2];
        count = re(pattern, string);
        printf("Total matches for pattern '%s' in string '%s': %d\n", pattern, string, count);
        return 0;
    }
}
#endif