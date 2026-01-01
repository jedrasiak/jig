#ifndef FILTER_H
#define FILTER_H

/**
 * Filter function that processes filepaths
 * Accepts filepaths from:
 * - First positional argument (argc >= 3, argv[2])
 * - stdin (piped input, one path per line)
 * Returns 0 on success, non-zero on error
 */
int filter(int argc, char **argv);

#endif /* FILTER_H */
