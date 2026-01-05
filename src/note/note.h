#ifndef NOTE_H
#define NOTE_H

/**
 * Create note scaffold with directory and markdown file(s)
 *
 * Creates a directory (slug from title) with one or more markdown files.
 * Each file contains template content with replaced placeholders.
 *
 * Usage:
 *   jig note <TITLE> [OPTIONS]
 *
 * Arguments:
 *   TITLE                Mandatory title for the note
 *
 * Options:
 *   -h, --help           Display help and exit
 *   -l, --lang LANGS     Comma-separated languages (creates index.LANG.md)
 *   -t, --template PATH  Use custom template file
 *
 * Template Placeholders:
 *   {{id}}              Replaced with generated UUID v7
 *   {{title}}           Replaced with provided title
 *   {{slug}}            Replaced with slugified title
 *
 * Default template (if no -t flag):
 *   ---
 *   id: {{id}}
 *   ---
 *
 *   # {{title}}
 *
 * Returns:
 *   0 on success, 1 on error
 *
 * Examples:
 *   jig note "My New Note"
 *   jig note "My Note" -l "en,pl"
 *   jig note "My Note" -t template.md
 */
int note(int argc, char **argv);

#endif /* NOTE_H */
