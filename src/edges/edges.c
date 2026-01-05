#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <regex.h>

#include "edges.h"

/**
 * Display help message
 */
static void help(void) {
    printf("Usage: jig edges [OPTIONS]\n");
    printf("       jig-edges [OPTIONS]\n");
    printf("\n");
    printf("Extract and list edges (relationships) between nodes.\n");
    printf("\n");
    printf("Reads node CSV from stdin and outputs edge information as CSV.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help          Display this help and exit\n");
    printf("\n");
    printf("Output Format:\n");
    printf("  CSV with columns: src_id,src_title,dst_id,dst_title,label,src_path,dst_path\n");
    printf("\n");
    printf("Examples:\n");
    printf("  jig find . -p \"\\.md$\" | jig filter | jig nodes | jig edges\n");
    printf("  jig-find . | jig-filter | jig-nodes | jig-edges\n");
}

/**
 * Extract label from query parameter in URL
 * Input: "/path/file.md?label=parent&other=value"
 * Output: "parent" (allocated string)
 * Returns NULL if no label parameter found
 */
static char* extract_label_from_url(const char *url) {
    // Find start of query string
    char *query_start = strchr(url, '?');
    if (query_start == NULL) return NULL;

    // Find label parameter
    char *label_param = strstr(query_start, "label=");
    if (label_param == NULL) return NULL;

    // Move past "label="
    char *label_start = label_param + 6;

    // Find end of label value (& or end of string)
    char *label_end = strchr(label_start, '&');
    size_t label_len = label_end ? (size_t)(label_end - label_start) : strlen(label_start);

    if (label_len == 0) return NULL;

    // Allocate and copy
    char *label = malloc(label_len + 1);
    if (label == NULL) return NULL;
    strncpy(label, label_start, label_len);
    label[label_len] = '\0';

    return label;
}

/**
 * Extract all markdown links from content
 * Pattern: [text](url) or [text](url?label=X)
 * Returns LinkList* or NULL on error
 */
static LinkList* extract_links_from_content(const char *content) {
    regex_t rgx;
    regmatch_t matches[3];

    // Compile regex for markdown links: \[([^][]*)\]\(([^)]+)\)
    char *pattern = "\\[([^][]*)\\]\\(([^)]+)\\)";
    if (regcomp(&rgx, pattern, REG_EXTENDED) != 0) {
        return NULL;
    }

    LinkList *list = malloc(sizeof(LinkList));
    if (list == NULL) {
        regfree(&rgx);
        return NULL;
    }
    list->items = NULL;
    list->count = 0;

    // Find all matches
    const char *cursor = content;
    while (regexec(&rgx, cursor, 3, matches, 0) == 0) {
        // Extract text (group 1)
        int text_len = matches[1].rm_eo - matches[1].rm_so;
        char *text = malloc(text_len + 1);
        if (text == NULL) {
            regfree(&rgx);
            for (int i = 0; i < list->count; i++) {
                free(list->items[i].text);
                free(list->items[i].path);
                free(list->items[i].label);
            }
            free(list->items);
            free(list);
            return NULL;
        }
        strncpy(text, cursor + matches[1].rm_so, text_len);
        text[text_len] = '\0';

        // Extract URL (group 2)
        int url_len = matches[2].rm_eo - matches[2].rm_so;
        char *url = malloc(url_len + 1);
        if (url == NULL) {
            free(text);
            regfree(&rgx);
            for (int i = 0; i < list->count; i++) {
                free(list->items[i].text);
                free(list->items[i].path);
                free(list->items[i].label);
            }
            free(list->items);
            free(list);
            return NULL;
        }
        strncpy(url, cursor + matches[2].rm_so, url_len);
        url[url_len] = '\0';

        // Extract label from URL or use default
        char *label = extract_label_from_url(url);
        if (label == NULL) {
            label = strdup("link");
        }

        // Add to list (realloc, store values)
        Link *tmp = realloc(list->items, (list->count + 1) * sizeof(Link));
        if (tmp == NULL) {
            free(text);
            free(url);
            free(label);
            regfree(&rgx);
            for (int i = 0; i < list->count; i++) {
                free(list->items[i].text);
                free(list->items[i].path);
                free(list->items[i].label);
            }
            free(list->items);
            free(list);
            return NULL;
        }
        list->items = tmp;

        list->items[list->count].text = text;
        list->items[list->count].path = url;
        list->items[list->count].label = label;
        list->count++;

        // Move cursor past this match
        cursor += matches[0].rm_eo;
    }

    regfree(&rgx);
    return list;
}

/**
 * Extract all markdown links from a file
 */
static LinkList* extract_links_from_file(const char *filepath) {
    FILE *fptr = fopen(filepath, "r");
    if (fptr == NULL) return NULL;

    // Read file size
    fseek(fptr, 0L, SEEK_END);
    long filesize = ftell(fptr);
    fseek(fptr, 0L, SEEK_SET);

    // Read content
    char *content = malloc(filesize + 1);
    if (content == NULL) {
        fclose(fptr);
        return NULL;
    }

    fread(content, 1, filesize, fptr);
    content[filesize] = '\0';
    fclose(fptr);

    // Extract links
    LinkList *links = extract_links_from_content(content);
    free(content);

    return links;
}

/**
 * Free link list memory
 */
static void free_links(LinkList *list) {
    if (list == NULL) return;
    for (int i = 0; i < list->count; i++) {
        free(list->items[i].text);
        free(list->items[i].path);
        free(list->items[i].label);
    }
    free(list->items);
    free(list);
}

/**
 * Build edges from nodes by extracting all links from files
 * Returns EdgeList* or NULL on error
 */
EdgeList* build_edges_from_nodes(NodeList *nodes) {
    EdgeList *edges = malloc(sizeof(EdgeList));
    if (edges == NULL) return NULL;
    edges->items = NULL;
    edges->count = 0;

    // For each source node
    for (int i = 0; i < nodes->count; i++) {
        Node *src_node = &nodes->items[i];

        // Extract all links from this file
        LinkList *links = extract_links_from_file(src_node->path);
        if (links == NULL) continue;

        // For each link found
        for (int j = 0; j < links->count; j++) {
            Link *link = &links->items[j];

            // Strip query string from link path for matching
            char *query_pos = strchr(link->path, '?');
            char link_path_clean[1024];
            if (query_pos != NULL) {
                size_t path_len = query_pos - link->path;
                strncpy(link_path_clean, link->path, path_len);
                link_path_clean[path_len] = '\0';
            } else {
                strncpy(link_path_clean, link->path, sizeof(link_path_clean) - 1);
                link_path_clean[sizeof(link_path_clean) - 1] = '\0';
            }

            // Find target node by matching path (using strstr)
            Node *dst_node = NULL;
            for (int k = 0; k < nodes->count; k++) {
                if (strstr(nodes->items[k].path, link_path_clean) != NULL) {
                    dst_node = &nodes->items[k];
                    break;
                }
            }

            if (dst_node == NULL) continue;  // Target not found

            // Create edge
            Edge *tmp = realloc(edges->items, (edges->count + 1) * sizeof(Edge));
            if (tmp == NULL) {
                free_links(links);
                free_edges(edges);
                return NULL;
            }
            edges->items = tmp;

            Edge *edge = &edges->items[edges->count];
            edge->src = src_node;
            edge->dst = dst_node;
            edge->label = strdup(link->label);  // Copy label from link

            edges->count++;
        }

        free_links(links);
    }

    return edges;
}

/**
 * Print edges as CSV to stdout
 * Format: src_id,src_title,dst_id,dst_title,label,src_path,dst_path
 */
void print_edges_csv(EdgeList *edges) {
    printf("src_id,src_title,dst_id,dst_title,label,src_path,dst_path\n");
    for (int i = 0; i < edges->count; i++) {
        Edge *edge = &edges->items[i];

        printf("%s,%s,%s,%s,%s,%s,%s\n",
               edge->src->id[0] ? edge->src->id : "",
               edge->src->title ? edge->src->title : "",
               edge->dst->id[0] ? edge->dst->id : "",
               edge->dst->title ? edge->dst->title : "",
               edge->label ? edge->label : "",
               edge->src->path ? edge->src->path : "",
               edge->dst->path ? edge->dst->path : "");
    }
}

/**
 * Free edge list memory
 */
void free_edges(EdgeList *edges) {
    if (edges == NULL) return;

    for (int i = 0; i < edges->count; i++) {
        free(edges->items[i].label);
    }
    free(edges->items);
    free(edges);
}

/**
 * Parse a CSV line into a Node
 * Expected format: id,title,path
 * Returns 0 on success, 1 on error
 */
static int parse_node_csv_line(Node *node, const char *line) {
    char *line_copy = strdup(line);
    if (line_copy == NULL) {
        return 1;
    }

    // Initialize node fields
    node->id[0] = '\0';
    node->title = NULL;
    node->path = NULL;

    // Parse CSV fields
    char *token;
    char *saveptr;
    int field = 0;

    token = strtok_r(line_copy, ",", &saveptr);
    while (token != NULL && field < 3) {
        switch (field) {
            case 0:  // id
                strncpy(node->id, token, 36);
                node->id[36] = '\0';
                break;
            case 1:  // title
                if (strlen(token) > 0) {
                    node->title = strdup(token);
                }
                break;
            case 2:  // path
                if (strlen(token) > 0) {
                    node->path = strdup(token);
                }
                break;
        }
        field++;
        token = strtok_r(NULL, ",", &saveptr);
    }

    free(line_copy);
    return 0;
}

/**
 * Read node CSV from stdin and build NodeList
 * Returns NodeList* or NULL on error
 */
static NodeList* read_nodes_from_csv(void) {
    char line[PATH_MAX];
    int is_header = 1;

    NodeList *list = malloc(sizeof(NodeList));
    if (list == NULL) {
        fprintf(stderr, "Failed to allocate NodeList\n");
        return NULL;
    }
    list->items = NULL;
    list->count = 0;

    while (fgets(line, sizeof(line), stdin) != NULL) {
        // Skip header line
        if (is_header) {
            is_header = 0;
            continue;
        }

        // Remove trailing newline
        line[strcspn(line, "\n")] = '\0';

        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }

        // Grow array
        Node *tmp = realloc(list->items, (list->count + 1) * sizeof(Node));
        if (tmp == NULL) {
            fprintf(stderr, "Failed to allocate memory for nodes\n");
            // Free existing nodes
            for (int i = 0; i < list->count; i++) {
                free(list->items[i].title);
                free(list->items[i].path);
            }
            free(list->items);
            free(list);
            return NULL;
        }
        list->items = tmp;

        // Parse node from CSV line
        Node *node = &list->items[list->count];
        if (parse_node_csv_line(node, line) != 0) {
            continue;  // Skip malformed lines
        }

        list->count++;
    }

    return list;
}

/**
 * Free nodes read from CSV
 */
static void free_csv_nodes(NodeList *list) {
    if (list == NULL) return;

    for (int i = 0; i < list->count; i++) {
        free(list->items[i].title);
        free(list->items[i].path);
    }
    free(list->items);
    free(list);
}

/**
 * Entry point for edges command
 * Expects node CSV on stdin, outputs edge CSV
 */
int edges(int argc, char **argv) {
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        }
    }

    // Read nodes from CSV stdin
    NodeList *nodes = read_nodes_from_csv();
    if (nodes == NULL) {
        return 1;
    }

    // Build edges from nodes
    EdgeList *edge_list = build_edges_from_nodes(nodes);
    if (edge_list == NULL) {
        free_csv_nodes(nodes);
        return 1;
    }

    // Print edges as CSV
    print_edges_csv(edge_list);

    // Cleanup
    free_edges(edge_list);
    free_csv_nodes(nodes);

    return 0;
}
