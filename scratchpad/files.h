#ifndef FILES_H
#define FILES_H

#define MAX_PATH 4096
#define MAX_DEPTH 100

extern char **text_files_list;
extern int text_files_count;
extern int text_files_capacity;

// Count how many text files exist (first pass)
int count_text_files(const char *path, int depth);

// Allocate the text_files_list array
void init_text_files_list(int count);

// Populate the text_files_list array (second pass)
int get_text_files(const char *path, int depth);

// Free the text_files_list array
void free_text_files_list();

#endif
