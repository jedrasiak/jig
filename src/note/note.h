#ifndef NOTE_H
#define NOTE_H

#define NOTE_MAX_ITEMS 64

/**
 * Link to another note
 */
typedef struct {
    char *label;    /* Link label (e.g., "parent") */
    char *path;     /* Resolved absolute path to target file */
    char *title;    /* Title extracted from target note */
} NoteLink;

/**
 * List of links for a note
 */
typedef struct {
    NoteLink items[NOTE_MAX_ITEMS];
    int count;
} NoteLinkList;

/**
 * Single note file to be created
 */
typedef struct {
    char *lang;     /* Language code (e.g., "en", "pl") or NULL for single note */
    char *title;    /* Note title */
    char *slug;     /* Slugified title */
    char *id;       /* UUID v7 */
    NoteLinkList links;
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
 * Free note link list memory
 */
void free_note_link_list(NoteLinkList *list);

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
