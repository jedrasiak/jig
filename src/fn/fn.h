#ifndef FN_H
#define FN_H

/**
 * Filter function that processes filepaths
 * Accepts filepaths from:
 * - First positional argument (argc >= 3, argv[2])
 * - stdin (piped input, one path per line)
 * Returns 0 on success, non-zero on error
 */
int fn(int argc, char **argv);

#endif /* FN_H */
