#ifndef NOTE_H
#define NOTE_H

#define NOTE_MAX_ITEMS 64

/**
 * Single note file to be created
 */
typedef struct {
    char *lang;     /* Language code (e.g., "en", "pl") or NULL for single note */
    char *title;    /* Note title */
    char *slug;     /* Slugified title */
    char *id;       /* UUID v7 */
} Note;

/**
 * List of notes to create
 */
typedef struct {
    Note items[NOTE_MAX_ITEMS];
    int count;
} NoteList;

/**
 * Folder containing notes
 */
typedef struct {
    char *name;     /* Original folder name from argument */
    char *slug;     /* Slugified folder name (actual directory) */
} NoteFolder;

/**
 * Free note list memory
 */
void free_note_list(NoteList *list);

/**
 * Free folder memory
 */
void free_note_folder(NoteFolder *folder);

/**
 * Entry point for note command
 */
int note(int argc, char **argv);

#endif /* NOTE_H */
