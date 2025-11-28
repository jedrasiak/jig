#include <stdio.h>
#include <regex.h>
#include <string.h>

int main() {
    // Step 1: Declare a regex object
    regex_t regex;
    int result;
    
    // Step 2: Compile the regex pattern
    // Pattern: "hello" - matches the literal string "hello"
    result = regcomp(&regex, "hello", REG_EXTENDED);
    
    if (result != 0) {
        fprintf(stderr, "Could not compile regex\n");
        return 1;
    }
    
    // Step 3: Execute the regex against test strings
    const char *test1 = "hello world";
    const char *test2 = "goodbye world";
    
    printf("Testing: '%s'\n", test1);
    result = regexec(&regex, test1, 0, NULL, 0);
    if (result == 0) {
        printf("  ✓ Match found\n");
    } else if (result == REG_NOMATCH) {
        printf("  ✗ No match\n");
    }
    
    printf("\nTesting: '%s'\n", test2);
    result = regexec(&regex, test2, 0, NULL, 0);
    if (result == 0) {
        printf("  ✓ Match found\n");
    } else if (result == REG_NOMATCH) {
        printf("  ✗ No match\n");
    }
    
    // Step 4: Free the compiled regex (CRITICAL - don't be sloppy)
    regfree(&regex);
    
    return 0;
}