#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "files.h"
#include "re.h"

typedef struct {
    char *file_path;
    int score;
    int rank;
} SearchResult;

int compare_results(const void *a, const void *b);
int search(const char *path, const char *algorithm, const char *format, const char *query);

int compare_results(const void *a, const void *b) {
    const SearchResult *result_a = (const SearchResult *)a;
    const SearchResult *result_b = (const SearchResult *)b;
    // descending order: higher scores first
    return result_b->score - result_a->score;
}

int search(const char *path, const char *algorithm, const char *format, const char *query) {
    // Debugging Information
    /*
    printf("path: %s\n", path);
    printf("algorithm: %s\n", algorithm);
    printf("format: %s\n", format);
    printf("query: %s\n", query);
    printf("---\n\n");
    */

    // validate inputs
    if (!path || !algorithm || !format || !query) {
        fprintf(stderr, "Error: Invalid arguments to search function\n");
        return 1;
    }

    if (strcmp(algorithm, "re") != 0) {
        fprintf(stderr, "Error: Unknown algorithm '%s'\n", algorithm);
        return 1;
    }

    if (strcmp(format, "table") != 0 && strcmp(format, "csv") != 0) {
        fprintf(stderr, "Error: Unknown format '%s'\n", format);
        return 1;
    }

    // regex algorithm
    if (strcmp(algorithm, "re") == 0) {
        text_files_count = 0;
        int errors = get_text_files(path, 0);

        if (errors > 0) {
            fprintf(stderr, "\nCompleted with %d errors\n", errors);
            return 1;
        } else {
            // allocate results array
            SearchResult *results = malloc(text_files_count * sizeof(SearchResult));
            if (!results) {
                fprintf(stderr, "Error: Could not allocate memory for results\n");
                return 1;
            }

            int results_count = 0;

            for (int i = 0; i < text_files_count; i++) {
                char *file_path = text_files_list[i];

                // open file
                FILE *file = fopen(file_path, "r");
                if (!file) {
                    fprintf(stderr, "Error: Could not open file '%s'\n", file_path);
                    continue;
                }

                // get file size
                fseek(file, 0, SEEK_END);
                long file_size = ftell(file);
                fseek(file, 0, SEEK_SET);

                // allocate buffer and read content
                char *content = malloc(file_size + 1);
                if (!content) {
                    fprintf(stderr, "Error: Could not allocate memory for '%s'\n", file_path);
                    fclose(file);
                    continue;
                }

                fread(content, 1, file_size, file);
                content[file_size] = '\0';
                fclose(file);

                // search for matches
                int count = re(query, content);

                // collect result if matches found
                if (count > 0) {
                    results[results_count].file_path = file_path;
                    results[results_count].score = count;
                    results[results_count].rank = 0;
                    results_count++;
                }

                // cleanup
                free(content);
            }

            // sort results by count (descending)
            qsort(results, results_count, sizeof(SearchResult), compare_results);

            // assign ranks
            for (int i = 0; i < results_count; i++) {
                results[i].rank = i + 1;
            }

            // return results in specified format
            if (strcmp(format, "table") == 0) {
                printf("%-5s %-10s %s", "Rank", "Score", "Path\n");
                printf("---------------------------------------------------------------\n");
                for (int i = 0; i < results_count; i++) {
                    printf("%-5d %-10d %s\n", results[i].rank, results[i].score, results[i].file_path);
                }
            } else if (strcmp(format, "csv") == 0) {
                printf("rank,score,path\n");
                for (int i = 0; i < results_count; i++) {
                    printf("%d,%d,%s\n", results[i].rank, results[i].score, results[i].file_path);
                }
            }

            // cleanup
            free(results);
        }

        return 0;
    }

    // should never reach here due to validation above
    return 1;
}


