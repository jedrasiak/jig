#ifndef FILES_H
#define FILES_H

#define MAX_PATH 4096
#define MAX_FILES 1024
#define MAX_DEPTH 100

extern char markdown_files_list[MAX_FILES][MAX_PATH];
extern int markdown_files_count;

int get_markdown_files(const char *path, int depth);

#endif
