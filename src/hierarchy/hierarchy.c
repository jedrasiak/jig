#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <regex.h>

#include "hierarchy.h"
#include "nodes/nodes.h"
#include "edges/edges.h"
#include "slugify/slugify.h"

/**
 * Display help message
 */
static void help(void) {
    printf("Usage: jig hierarchy [OPTIONS]\n");
    printf("       jig-hierarchy [OPTIONS]\n");
    printf("\n");
    printf("Generate hierarchy data mapping each note's slug to its ancestor chain.\n");
    printf("\n");
    printf("Reads file paths from stdin and outputs YAML format suitable for Hugo's\n");
    printf("data/ directory. The output maps each note's slug to an ordered list of\n");
    printf("ancestor slugs (from immediate parent to root).\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help          Display this help and exit\n");
    printf("\n");
    printf("Output Format:\n");
    printf("  YAML with structure:\n");
    printf("    <slug>:\n");
    printf("      ancestors:\n");
    printf("        - <parent-slug>\n");
    printf("        - <grandparent-slug>\n");
    printf("        - ...\n");
    printf("\n");
    printf("Examples:\n");
    printf("  jig find . -p \"\\.en.md$\" | jig filter | jig hierarchy\n");
    printf("  jig find . -p \"\\.en.md$\" | jig filter | jig hierarchy > data/hierarchy_en.yaml\n");
}

/**
 * Extract folder name from file path
 * Path like "/binary/index.en.md" returns "binary"
 */
static char* extract_folder_name(const char *filepath) {
    if (filepath == NULL) return NULL;

    char *path_copy = strdup(filepath);
    if (path_copy == NULL) return NULL;

    // Remove trailing filename (find last /)
    char *last_slash = strrchr(path_copy, '/');
    if (last_slash != NULL) {
        *last_slash = '\0';
    }

    // Find the folder name (last component)
    char *folder_start = strrchr(path_copy, '/');
    if (folder_start != NULL) {
        folder_start++;
    } else {
        folder_start = path_copy;
    }

    char *folder = strdup(folder_start);
    free(path_copy);

    return folder;
}

/**
 * Parse slug field from file's frontmatter
 * Returns allocated string or NULL if not found
 */
static char* parse_slug_from_file(const char *filepath) {
    FILE *fptr = fopen(filepath, "r");
    if (fptr == NULL) return NULL;

    fseek(fptr, 0L, SEEK_END);
    long filesize = ftell(fptr);
    fseek(fptr, 0L, SEEK_SET);

    char *content = malloc(filesize + 1);
    if (content == NULL) {
        fclose(fptr);
        return NULL;
    }

    fread(content, 1, filesize, fptr);
    content[filesize] = '\0';
    fclose(fptr);

    // Find slug field using regex
    regex_t rgx;
    regmatch_t matches[2];
    char *pattern = "^slug: (.*)$";

    if (regcomp(&rgx, pattern, REG_EXTENDED | REG_NEWLINE) != 0) {
        free(content);
        return NULL;
    }

    char *slug = NULL;
    if (regexec(&rgx, content, 2, matches, 0) == 0) {
        int start = matches[1].rm_so;
        int end = matches[1].rm_eo;
        int len = end - start;

        slug = malloc(len + 1);
        if (slug != NULL) {
            strncpy(slug, content + start, len);
            slug[len] = '\0';
        }
    }

    regfree(&rgx);
    free(content);
    return slug;
}

/**
 * Get slug for a node
 * First checks for slug in frontmatter, falls back to folder name
 */
static char* get_node_slug(const char *filepath) {
    // Try slug from frontmatter first
    char *slug = parse_slug_from_file(filepath);
    if (slug != NULL) {
        return slug;
    }

    // Fallback to folder name
    return extract_folder_name(filepath);
}

/**
 * Find parent node of a given node using edges
 * Returns the parent node or NULL if no parent
 */
static Node* find_parent_node(Node *node, EdgeList *edges) {
    for (int i = 0; i < edges->count; i++) {
        if (edges->items[i].src == node &&
            edges->items[i].label != NULL &&
            strcmp(edges->items[i].label, "parent") == 0) {
            return edges->items[i].dst;
        }
    }
    return NULL;
}

/**
 * Build ancestor chain for a node
 * Returns array of ancestor slugs (parent first, root last)
 * Sets *count to number of ancestors
 */
static char** build_ancestor_chain(Node *node, EdgeList *edges, int *count) {
    *count = 0;
    char **ancestors = NULL;

    Node *current = node;
    Node *parent = find_parent_node(current, edges);

    while (parent != NULL) {
        // Get slug for parent node
        char *parent_slug = get_node_slug(parent->path);
        if (parent_slug == NULL) {
            parent = find_parent_node(parent, edges);
            continue;
        }

        // Grow array
        char **tmp = realloc(ancestors, (*count + 1) * sizeof(char*));
        if (tmp == NULL) {
            free(parent_slug);
            // Free existing ancestors on error
            for (int i = 0; i < *count; i++) {
                free(ancestors[i]);
            }
            free(ancestors);
            *count = 0;
            return NULL;
        }
        ancestors = tmp;
        ancestors[*count] = parent_slug;
        (*count)++;

        // Move up the tree
        current = parent;
        parent = find_parent_node(current, edges);
    }

    return ancestors;
}

/**
 * Print hierarchy entry in YAML format
 */
static void print_hierarchy_yaml(const char *slug, char **ancestors, int ancestor_count) {
    printf("%s:\n", slug);
    printf("  ancestors:");
    if (ancestor_count == 0) {
        printf(" []\n");
    } else {
        printf("\n");
        for (int i = 0; i < ancestor_count; i++) {
            printf("    - %s\n", ancestors[i]);
        }
    }
}

/**
 * Entry point for hierarchy command
 */
int hierarchy(int argc, char **argv) {
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        }
    }

    // Initialize node parser
    if (init_node_parser() != 0) {
        return 1;
    }

    // Build nodes from stdin
    NodeList *nodes = build_nodes_from_stdin();
    if (nodes == NULL) {
        cleanup_node_parser();
        return 1;
    }

    // Build edges from nodes
    EdgeList *edges = build_edges_from_nodes(nodes);
    if (edges == NULL) {
        free_nodes(nodes);
        cleanup_node_parser();
        return 1;
    }

    // For each node, build and print hierarchy entry
    for (int i = 0; i < nodes->count; i++) {
        Node *node = &nodes->items[i];

        // Get slug for this node
        char *slug = get_node_slug(node->path);
        if (slug == NULL) continue;

        // Build ancestor chain
        int ancestor_count = 0;
        char **ancestors = build_ancestor_chain(node, edges, &ancestor_count);

        // Print YAML entry
        print_hierarchy_yaml(slug, ancestors, ancestor_count);

        // Cleanup
        for (int j = 0; j < ancestor_count; j++) {
            free(ancestors[j]);
        }
        free(ancestors);
        free(slug);
    }

    // Cleanup
    free_edges(edges);
    free_nodes(nodes);
    cleanup_node_parser();

    return 0;
}
