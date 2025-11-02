#ifndef RE_H
#define RE_H

/**
 * Count the number of matches for a regex pattern in a string.
 *
 * @param pattern The regular expression pattern (POSIX extended regex)
 * @param string The string to search in
 * @return Number of matches found, or -1 on error
 */
int re(const char *pattern, const char *string);

#endif // RE_H
