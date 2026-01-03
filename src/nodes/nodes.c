#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <regex.h>

#include "nodes.h"

// Global compiled regexes (static - internal only)
static regex_t rgx_link;
static regex_t rgx_id;
static regex_t rgx_title;
static int regexes_compiled = 0;

/**
 * Display help message
 */
static void help(void) {
    printf("Usage: jig nodes [OPTIONS]\n");
    printf("       jig-nodes [OPTIONS]\n");
    printf("\n");
    printf("Extract and list nodes from note files.\n");
    printf("\n");
    printf("Reads file paths from stdin and outputs node information as CSV.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help          Display this help and exit\n");
    printf("\n");
    printf("Output Format:\n");
    printf("  CSV with columns: id,title,path,link\n");
    printf("\n");
    printf("Examples:\n");
    printf("  jig find . -p \"\\.md$\" | jig filter | jig nodes\n");
    printf("  jig-find . | jig-filter | jig-nodes\n");
}

/**
 * Initialize regex patterns for node parsing
 * Must be called before parse_node()
 * Returns 0 on success, non-zero on error
 */
int init_node_parser(void) {
    if (regexes_compiled) return 0;

    char *rgx_link_pattern = "\\[.*\\]\\((.*)\\?label=parent\\)";
    char *rgx_id_pattern = "id: (.*)";
    char *rgx_title_pattern = "title: (.*)";
    int rgx_result;

    rgx_result = regcomp(&rgx_link, rgx_link_pattern, REG_EXTENDED | REG_ICASE | REG_NEWLINE);
    if (rgx_result != 0) {
        fprintf(stderr, "Could not compile regex: rgx_link_pattern\n");
        return 1;
    }

    rgx_result = regcomp(&rgx_id, rgx_id_pattern, REG_EXTENDED | REG_ICASE | REG_NEWLINE);
    if (rgx_result != 0) {
        fprintf(stderr, "Could not compile regex: rgx_id_pattern\n");
        return 1;
    }

    rgx_result = regcomp(&rgx_title, rgx_title_pattern, REG_EXTENDED | REG_ICASE | REG_NEWLINE);
    if (rgx_result != 0) {
        fprintf(stderr, "Could not compile regex: rgx_title_pattern\n");
        return 1;
    }

    regexes_compiled = 1;
    return 0;
}

/**
 * Free regex patterns
 * Should be called when done parsing nodes
 */
void cleanup_node_parser(void) {
    if (!regexes_compiled) return;

    regfree(&rgx_link);
    regfree(&rgx_id);
    regfree(&rgx_title);

    regexes_compiled = 0;
}

/**
 * Parse node fields (id, title, link) from file
 * Must call init_node_parser() first
 */
void parse_node(Node *node, const char *filepath) {
    // Initialize fields to NULL/empty
    node->id[0] = '\0';
    node->link = NULL;
    node->title = NULL;

    FILE *fptr = fopen(filepath, "r");
    if (fptr == NULL) {
        return;  // Skip file if can't open
    }

    // Read file size
    fseek(fptr, 0L, SEEK_END);
    long filesize = ftell(fptr);
    fseek(fptr, 0L, SEEK_SET);

    // Read file content
    char *filecontent = malloc(filesize + 1);
    if (filecontent == NULL) {
        fclose(fptr);
        return;
    }

    for (long j = 0; j < filesize; j++) {
        filecontent[j] = fgetc(fptr);
    }
    filecontent[filesize] = '\0';
    fclose(fptr);

    // Parse ID
    regmatch_t rgx_id_matches[2];
    int rgx_result = regexec(&rgx_id, filecontent, 2, rgx_id_matches, 0);
    if (rgx_result == 0) {
        int match_start = rgx_id_matches[1].rm_so;
        int match_end = rgx_id_matches[1].rm_eo;
        int match_length = match_end - match_start;
        if (match_length > 36) {
            match_length = 36;
        }
        strncpy(node->id, filecontent + match_start, match_length);
        node->id[match_length] = '\0';
    }

    // Parse link
    regmatch_t rgx_link_matches[2];
    rgx_result = regexec(&rgx_link, filecontent, 2, rgx_link_matches, 0);
    if (rgx_result == 0) {
        int match_start = rgx_link_matches[1].rm_so;
        int match_end = rgx_link_matches[1].rm_eo;
        int match_length = match_end - match_start;

        node->link = malloc(match_length + 1);
        if (node->link != NULL) {
            strncpy(node->link, filecontent + match_start, match_length);
            node->link[match_length] = '\0';
        }
    }

    // Parse title
    regmatch_t rgx_title_matches[2];
    rgx_result = regexec(&rgx_title, filecontent, 2, rgx_title_matches, 0);
    if (rgx_result == 0) {
        int match_start = rgx_title_matches[1].rm_so;
        int match_end = rgx_title_matches[1].rm_eo;
        int match_length = match_end - match_start;

        node->title = malloc(match_length + 1);
        if (node->title != NULL) {
            strncpy(node->title, filecontent + match_start, match_length);
            node->title[match_length] = '\0';
        }
    }

    free(filecontent);
}

/**
 * Add single node to list from filepath
 * Caller must have called init_node_parser() first
 */
int add_node(NodeList *list, const char *filepath) {
    // Grow array by one item
    Node *tmp = realloc(list->items, (list->count + 1) * sizeof(Node));
    if (tmp == NULL) {
        fprintf(stderr, "Failed to allocate memory for nodes\n");
        return 1;
    }
    list->items = tmp;

    // Create new node
    Node *node = &list->items[list->count];

    // Allocate and copy path
    node->path = malloc(strlen(filepath) + 1);
    if (node->path == NULL) {
        fprintf(stderr, "Failed to allocate memory for path\n");
        return 1;
    }
    strcpy(node->path, filepath);

    // Parse node fields from file
    parse_node(node, filepath);

    list->count++;
    return 0;
}

/**
 * Read filepaths from stdin and build node list
 * Automatically calls init_node_parser() and cleanup_node_parser()
 * Returns NodeList* or NULL on error
 */
NodeList* build_nodes_from_stdin(void) {
    char filepath[PATH_MAX];

    NodeList *list = malloc(sizeof(NodeList));
    if (list == NULL) {
        fprintf(stderr, "Failed to allocate NodeList\n");
        return NULL;
    }
    list->items = NULL;
    list->count = 0;

    while (fgets(filepath, sizeof(filepath), stdin) != NULL) {
        // Remove trailing newline
        filepath[strcspn(filepath, "\n")] = '\0';

        // Add to node list
        add_node(list, filepath);
    }

    return list;
}

/**
 * Print nodes as CSV to stdout
 * Format: id,title,path,link
 */
void print_nodes_csv(NodeList *list) {
    printf("id,title,path,link\n");
    for (int i = 0; i < list->count; i++) {
        printf("%s,%s,%s,%s\n",
               list->items[i].id[0] ? list->items[i].id : "",
               list->items[i].title ? list->items[i].title : "",
               list->items[i].path ? list->items[i].path : "",
               list->items[i].link ? list->items[i].link : "");
    }
}

/**
 * Free node list memory
 */
void free_nodes(NodeList *list) {
    if (list == NULL) return;

    for (int i = 0; i < list->count; i++) {
        free(list->items[i].path);
        free(list->items[i].link);
        free(list->items[i].title);
    }
    free(list->items);
    free(list);
}

/**
 * Entry point for nodes command
 */
int nodes(int argc, char **argv) {
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        }
    }

    // Initialize parser
    if (init_node_parser() != 0) {
        return 1;
    }

    // Build nodes from stdin
    NodeList *nodes_list = build_nodes_from_stdin();
    if (nodes_list == NULL) {
        cleanup_node_parser();
        return 1;
    }

    // Print as CSV
    print_nodes_csv(nodes_list);

    // Cleanup
    free_nodes(nodes_list);
    cleanup_node_parser();

    return 0;
}
