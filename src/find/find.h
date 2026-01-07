#ifndef FIND_H
#define FIND_H

/**
 * Find files recursively in directory tree
 * Usage:
 *   jig find <directory> [-p|--pattern <regex>]
 *
 * Recursively traverses directory structure (max 100 levels)
 * Skips hidden files (starting with .) and symbolic links
 * Optional regex pattern filtering with -p/--pattern flag
 * Outputs full file paths (one per line) to stdout
 *
 * Returns 0 on success, non-zero on error
 */
int find(int argc, char **argv);

#endif /* FIND_H */