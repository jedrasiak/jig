#ifndef HIERARCHY_H
#define HIERARCHY_H

/**
 * Represents a node's ancestry information for hierarchy output
 */
typedef struct {
    char *slug;           // Node's slug (derived from path or title)
    char **ancestors;     // Array of ancestor slugs (parent to root order)
    int ancestor_count;   // Number of ancestors
} HierarchyEntry;

/**
 * Dynamic array of hierarchy entries
 */
typedef struct {
    HierarchyEntry *items;
    int count;
} HierarchyList;

/**
 * Generate hierarchy data mapping each note's slug to its ancestor chain.
 * Reads file paths from stdin (same as jig tree).
 * Outputs YAML format suitable for Hugo's data/ directory.
 *
 * Returns 0 on success, non-zero on error
 */
int hierarchy(int argc, char **argv);

#endif /* HIERARCHY_H */
