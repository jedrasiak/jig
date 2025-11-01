#ifndef SEARCH_H
#define SEARCH_H

/**
    * Searches for a query string within the graph's vertices.
    *
    * @param query The search query string
    * @param path The path to start the search from
    * @param algorithm The search algorithm to use (e.g., "BFS", "DFS")
    * @return 0 on success, 1 on failure
*/

int search(const char *query, const char *path, const char *algorithm);

#endif // SEARCH_H