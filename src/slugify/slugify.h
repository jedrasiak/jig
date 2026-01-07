#ifndef SLUGIFY_H
#define SLUGIFY_H

/**
 * Convert a string to a URL-friendly slug.
 *
 * Applies transformations:
 *   - Convert to lowercase
 *   - Remove non-ASCII characters
 *   - Replace spaces with hyphens
 *   - Remove all other non-alphanumeric characters
 *   - Collapse multiple hyphens into single hyphen
 *   - Trim leading/trailing hyphens
 *
 * Output contains only: a-z, 0-9, and hyphens (-)
 *
 * @param input String to slugify (NULL-terminated)
 * @return Newly allocated slug string
 *         Returns NULL on allocation error (error message to stderr)
 *         Returns empty string "" for empty input
 *         Caller is responsible for freeing the returned string
 *
 * Example usage:
 *   char *slug = slugify("Hello, World! 2024");
 *   if (slug != NULL) {
 *       printf("%s\n", slug);  // Output: hello-world-2024
 *       free(slug);
 *   }
 */
char *slugify(const char *input);

#endif /* SLUGIFY_H */
