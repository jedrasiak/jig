#ifndef TREE_H
#define TREE_H

/**
 * Tree function that processes filepaths
 * Accepts filepaths from:
 * - First positional argument (argc >= 3, argv[2])
 * - stdin (piped input, one path per line)
 * Returns 0 on success, non-zero on error
 */

 int tree(int argc, char **argv);

#endif /* TREE_H */