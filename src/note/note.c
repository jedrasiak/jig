#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include "note.h"
#include "../uuid/uuid.h"
#include "../slugify/slugify.h"

#define MAX_TEMPLATE_SIZE 10485760  /* 10MB */

/* ============================================================================
 * Help
 * ============================================================================ */

static void help(void) {
    printf("Usage: jig note <NAME> [OPTIONS]\n");
    printf("\n");
    printf("Create note scaffold with directory and markdown file(s).\n");
    printf("\n");
    printf("Arguments:\n");
    printf("  NAME               Folder name (slugified), also used as default title\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help             Display this help and exit\n");
    printf("  -n, --name LANG:TITLE  Language-specific title (can be repeated)\n");
    printf("  -t, --template PATH    Use custom template file\n");
    printf("\n");
    printf("Template Placeholders:\n");
    printf("  {{id}}             Replaced with generated UUID v7\n");
    printf("  {{title}}          Replaced with provided title\n");
    printf("  {{slug}}           Replaced with slugified title (per-language when using -n)\n");
    printf("\n");
    printf("Default template (if no -t flag):\n");
    printf("  ---\n");
    printf("  id: {{id}}\n");
    printf("  ---\n");
    printf("\n");
    printf("  # {{title}}\n");
    printf("\n");
    printf("Examples:\n");
    printf("  jig note \"My Note\"\n");
    printf("  jig note \"My Note\" -n \"en:My Note\" -n \"pl:Moja Notatka\"\n");
    printf("  jig note \"My Note\" -t template.md\n");
}

/* ============================================================================
 * Cleanup functions
 * ============================================================================ */

void free_note_list(NoteList *list) {
    for (int i = 0; i < list->count; i++) {
        free(list->items[i].lang);
        free(list->items[i].title);
        free(list->items[i].slug);
        free(list->items[i].id);
    }
    list->count = 0;
}

void free_note_folder(NoteFolder *folder) {
    free(folder->name);
    free(folder->slug);
    folder->name = NULL;
    folder->slug = NULL;
}

static void cleanup(char *template, NoteFolder *folder, NoteList *notes) {
    free(template);
    free_note_folder(folder);
    free_note_list(notes);
}

/* ============================================================================
 * Template functions
 * ============================================================================ */

static char *get_default_template(void) {
    const char *template = "---\nid: {{id}}\n---\n\n# {{title}}\n";
    return strdup(template);
}

static char *read_template_file(const char *filepath) {
    FILE *fptr = fopen(filepath, "r");
    if (fptr == NULL) {
        fprintf(stderr, "Error: Cannot open template file '%s'\n", filepath);
        return NULL;
    }

    fseek(fptr, 0L, SEEK_END);
    long filesize = ftell(fptr);
    fseek(fptr, 0L, SEEK_SET);

    if (filesize > MAX_TEMPLATE_SIZE) {
        fprintf(stderr, "Error: Template file too large (max 10MB)\n");
        fclose(fptr);
        return NULL;
    }

    char *content = malloc(filesize + 1);
    if (content == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(fptr);
        return NULL;
    }

    size_t read = fread(content, 1, filesize, fptr);
    content[read] = '\0';
    fclose(fptr);

    return content;
}

static char *replace_placeholder(const char *template, const char *placeholder, const char *value) {
    if (template == NULL || placeholder == NULL || value == NULL) {
        return NULL;
    }

    const char *pos = strstr(template, placeholder);
    if (pos == NULL) {
        return strdup(template);
    }

    size_t template_len = strlen(template);
    size_t placeholder_len = strlen(placeholder);
    size_t value_len = strlen(value);

    /* Count occurrences */
    int count = 0;
    const char *tmp = template;
    while ((tmp = strstr(tmp, placeholder)) != NULL) {
        count++;
        tmp += placeholder_len;
    }

    size_t new_size = template_len - (count * placeholder_len) + (count * value_len) + 1;
    char *result = malloc(new_size);
    if (result == NULL) {
        return NULL;
    }

    char *dst = result;
    const char *src = template;
    while ((pos = strstr(src, placeholder)) != NULL) {
        size_t before_len = pos - src;
        memcpy(dst, src, before_len);
        dst += before_len;
        memcpy(dst, value, value_len);
        dst += value_len;
        src = pos + placeholder_len;
    }
    strcpy(dst, src);

    return result;
}

/* ============================================================================
 * Content generation
 * ============================================================================ */

static char *generate_content(const char *template, Note *note) {
    char *step1 = replace_placeholder(template, "{{slug}}", note->slug);
    if (step1 == NULL) return NULL;

    char *step2 = replace_placeholder(step1, "{{title}}", note->title);
    free(step1);
    if (step2 == NULL) return NULL;

    char *step3 = replace_placeholder(step2, "{{id}}", note->id);
    free(step2);

    return step3;
}

/* ============================================================================
 * File creation
 * ============================================================================ */

static int create_file(const char *filepath, const char *content) {
    FILE *fptr = fopen(filepath, "r");
    if (fptr != NULL) {
        fclose(fptr);
        fprintf(stderr, "Error: File '%s' already exists\n", filepath);
        return 1;
    }

    fptr = fopen(filepath, "w");
    if (fptr == NULL) {
        fprintf(stderr, "Error: Cannot create file '%s'\n", filepath);
        return 1;
    }

    fprintf(fptr, "%s", content);
    fclose(fptr);
    return 0;
}

static int create_note_file(const char *folder_slug, const char *template, Note *note) {
    char *content = generate_content(template, note);
    if (content == NULL) {
        fprintf(stderr, "Error: Failed to generate content\n");
        return 1;
    }

    char filepath[PATH_MAX];
    if (note->lang != NULL) {
        snprintf(filepath, PATH_MAX, "%s/index.%s.md", folder_slug, note->lang);
    } else {
        snprintf(filepath, PATH_MAX, "%s/index.md", folder_slug);
    }

    int result = create_file(filepath, content);
    free(content);
    return result;
}

/* ============================================================================
 * Note building
 * ============================================================================ */

static int add_note(NoteList *list, const char *lang, const char *title) {
    if (list->count >= NOTE_MAX_ITEMS) {
        fprintf(stderr, "Error: Too many notes (max %d)\n", NOTE_MAX_ITEMS);
        return 1;
    }

    Note *note = &list->items[list->count];
    note->lang = lang ? strdup(lang) : NULL;
    note->title = strdup(title);
    note->slug = slugify(title);
    note->id = uuid(7);

    if (note->title == NULL || note->slug == NULL || note->id == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(note->lang);
        free(note->title);
        free(note->slug);
        free(note->id);
        return 1;
    }

    list->count++;
    return 0;
}

/* ============================================================================
 * Argument parsing
 * ============================================================================ */

typedef struct {
    char *folder_name;
    char *template_path;
    char *lang_titles[NOTE_MAX_ITEMS];  /* raw "LANG:TITLE" strings */
    int lang_title_count;
    int show_help;
} ParsedArgs;

static int parse_args(int argc, char **argv, ParsedArgs *args) {
    memset(args, 0, sizeof(ParsedArgs));

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            args->show_help = 1;
            return 0;
        }
        else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--name") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -n/--name requires LANG:TITLE argument\n");
                return 1;
            }
            if (args->lang_title_count >= NOTE_MAX_ITEMS) {
                fprintf(stderr, "Error: Too many -n/--name arguments\n");
                return 1;
            }
            args->lang_titles[args->lang_title_count++] = argv[++i];
        }
        else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--template") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -t/--template requires a file path\n");
                return 1;
            }
            args->template_path = argv[++i];
        }
        else if (argv[i][0] != '-') {
            args->folder_name = argv[i];
        }
        else {
            fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
            return 1;
        }
    }

    return 0;
}

static int parse_lang_title(const char *str, char **lang_out, char **title_out) {
    const char *colon = strchr(str, ':');
    if (colon == NULL) {
        fprintf(stderr, "Error: Invalid format '%s', expected LANG:TITLE\n", str);
        return 1;
    }

    size_t lang_len = colon - str;
    if (lang_len == 0) {
        fprintf(stderr, "Error: Empty language in '%s'\n", str);
        return 1;
    }

    const char *title = colon + 1;
    if (*title == '\0') {
        fprintf(stderr, "Error: Empty title in '%s'\n", str);
        return 1;
    }

    *lang_out = strndup(str, lang_len);
    *title_out = strdup(title);

    if (*lang_out == NULL || *title_out == NULL) {
        free(*lang_out);
        free(*title_out);
        return 1;
    }

    return 0;
}

/* ============================================================================
 * Main entry point
 * ============================================================================ */

int note(int argc, char **argv) {
    ParsedArgs args;
    NoteFolder folder = {0};
    NoteList notes = {0};
    char *template = NULL;

    /* Parse arguments */
    if (parse_args(argc, argv, &args) != 0) {
        help();
        return 1;
    }

    if (args.show_help) {
        help();
        return 0;
    }

    if (args.folder_name == NULL) {
        fprintf(stderr, "Error: Folder name is required\n");
        help();
        return 1;
    }

    /* Build folder */
    folder.name = strdup(args.folder_name);
    folder.slug = slugify(args.folder_name);
    if (folder.name == NULL || folder.slug == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        cleanup(template, &folder, &notes);
        return 1;
    }

    /* Build note list */
    if (args.lang_title_count == 0) {
        /* Single note using folder name as title */
        if (add_note(&notes, NULL, args.folder_name) != 0) {
            cleanup(template, &folder, &notes);
            return 1;
        }
    } else {
        /* Multiple notes from -n flags */
        for (int i = 0; i < args.lang_title_count; i++) {
            char *lang = NULL;
            char *title = NULL;
            if (parse_lang_title(args.lang_titles[i], &lang, &title) != 0) {
                cleanup(template, &folder, &notes);
                return 1;
            }
            int err = add_note(&notes, lang, title);
            free(lang);
            free(title);
            if (err != 0) {
                cleanup(template, &folder, &notes);
                return 1;
            }
        }
    }

    /* Check if directory exists */
    struct stat st;
    if (stat(folder.slug, &st) == 0) {
        fprintf(stderr, "Error: Directory '%s' already exists\n", folder.slug);
        cleanup(template, &folder, &notes);
        return 1;
    }

    /* Create directory */
    if (mkdir(folder.slug, 0755) != 0) {
        fprintf(stderr, "Error: Failed to create directory '%s'\n", folder.slug);
        cleanup(template, &folder, &notes);
        return 1;
    }

    /* Load template */
    if (args.template_path != NULL) {
        template = read_template_file(args.template_path);
    } else {
        template = get_default_template();
    }
    if (template == NULL) {
        cleanup(template, &folder, &notes);
        return 1;
    }

    /* Create note files */
    for (int i = 0; i < notes.count; i++) {
        if (create_note_file(folder.slug, template, &notes.items[i]) != 0) {
            cleanup(template, &folder, &notes);
            return 1;
        }
    }

    cleanup(template, &folder, &notes);
    return 0;
}
