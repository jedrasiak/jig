#ifndef NODES_H
#define NODES_H

/**
 * Represents a single node in the knowledge graph
 * Parsed from markdown file with YAML frontmatter
 *
 * Fields:
 *   id    - Unique identifier (max 36 chars, typically UUID)
 *   path  - Full file path to the source file
 *   title - Human-readable node title
 */
typedef struct {
    char id[37];
    char *path;
    char *title;
} Node;

/**
 * Dynamic array of nodes
 *
 * Fields:
 *   items - Array of Node structures
 *   count - Number of nodes in the array
 */
typedef struct {
    Node *items;
    int count;
} NodeList;

/**
 * Initialize regex patterns for node parsing
 * Must be called before parse_node()
 * Returns 0 on success, non-zero on error
 */
int init_node_parser(void);

/**
 * Free regex patterns
 * Should be called when done parsing nodes
 */
void cleanup_node_parser(void);

/**
 * Parse node fields (id, title) from file
 * Must call init_node_parser() first
 */
void parse_node(Node *node, const char *filepath);

/**
 * Add single node to list from filepath
 * Caller must have called init_node_parser() first
 */
int add_node(NodeList *list, const char *filepath);

/**
 * Read filepaths from stdin and build node list
 * Automatically calls init_node_parser() and cleanup_node_parser()
 * Returns NodeList* or NULL on error
 */
NodeList* build_nodes_from_stdin(void);

/**
 * Print nodes as CSV to stdout
 * Format: id,title,path
 */
void print_nodes_csv(NodeList *list);

/**
 * Free node list memory
 */
void free_nodes(NodeList *list);

/**
 * Entry point for nodes command
 */
int nodes(int argc, char **argv);

#endif /* NODES_H */