#ifndef TREE_H
#define TREE_H

/**
 * Visualize hierarchical tree structure from edges
 * Reads edge CSV from stdin with format:
 *   src_id,src_title,dst_id,dst_title,label,src_path,dst_path
 *
 * Builds parent-child relationships and renders ASCII tree
 * using Unicode box-drawing characters
 *
 * Returns 0 on success, non-zero on error
 */
int tree(int argc, char **argv);

#endif /* TREE_H */