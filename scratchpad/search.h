#ifndef SEARCH_H
#define SEARCH_H

/**
    * Searches for a query string within the graph's vertices.
    *
    * @param path The path to start the search from
    * @param algorithm The search algorithm to use (e.g., "BFS", "DFS")
    * @param format The output format (e.g., "table", "csv")
    * @param query The search query string
    * @return 0 on success, 1 on failure
*/

int search(const char *path, const char *algorithm, const char *format, const char *query);

#endif // SEARCH_H