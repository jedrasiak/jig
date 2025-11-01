#ifndef SEARCH_H
#define SEARCH_H

/**
    * Searches for a query string within the graph's vertices.
    *
    * @param query The search query string
    * @return 0 on success, 1 on failure
*/

int search(const char *query, const char *path);

#endif // SEARCH_H