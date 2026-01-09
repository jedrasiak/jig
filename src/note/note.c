#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include "note.h"
#include "../uuid/uuid.h"
#include "../slugify/slugify.h"
#include "../nodes/nodes.h"

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
    printf("  -l, --link LABEL:PATH  Link to existing note (can be repeated)\n");
    printf("  -n, --name LANG:TITLE  Language-specific title (can be repeated)\n");
    printf("  -t, --template PATH    Use custom template file\n");
    printf("\n");
    printf("Template Placeholders:\n");
    printf("  {{id}}             Replaced with generated UUID v7\n");
    printf("  {{title}}          Replaced with provided title\n");
    printf("  {{slug}}           Replaced with slugified title (per-language when using -n)\n");
    printf("  {{links}}          Replaced with formatted links list\n");
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
    printf("  jig note \"My Note\" -l \"parent:../other-note\"\n");
    printf("  jig note \"My Note\" -t template.md\n");
}

/* ============================================================================
 * Cleanup functions
 * ============================================================================ */

void free_note_link_list(NoteLinkList *list) {
    for (int i = 0; i < list->count; i++) {
        free(list->items[i].label);
        free(list->items[i].path);
        free(list->items[i].title);
    }
    list->count = 0;
}

void free_note_list(NoteList *list) {
    for (int i = 0; i < list->count; i++) {
        free(list->items[i].lang);
        free(list->items[i].title);
        free(list->items[i].slug);
        free(list->items[i].id);
        free_note_link_list(&list->items[i].links);
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
 * Link formatting
 * ============================================================================ */

static char *format_links(NoteLinkList *links) {
    if (links->count == 0) {
        return strdup("");
    }

    /* Calculate total size needed */
    size_t total_size = 0;
    for (int i = 0; i < links->count; i++) {
        /* Format: - "[title](path?label=label)\n" (no \n on last) */
        total_size += 5;  /* - "[ */
        total_size += strlen(links->items[i].title);
        total_size += 2;  /* ]( */
        total_size += strlen(links->items[i].path);
        total_size += 7;  /* ?label= */
        total_size += strlen(links->items[i].label);
        total_size += 2;  /* ") */
        if (i < links->count - 1) {
            total_size += 1;  /* \n between items */
        }
    }
    total_size += 1;  /* null terminator */

    char *result = malloc(total_size);
    if (result == NULL) {
        return NULL;
    }

    char *ptr = result;
    for (int i = 0; i < links->count; i++) {
        const char *fmt = (i < links->count - 1)
            ? "- \"[%s](%s?label=%s)\"\n"
            : "- \"[%s](%s?label=%s)\"";
        int written = sprintf(ptr, fmt,
                              links->items[i].title,
                              links->items[i].path,
                              links->items[i].label);
        ptr += written;
    }

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
    if (step3 == NULL) return NULL;

    char *links_str = format_links(&note->links);
    if (links_str == NULL) {
        free(step3);
        return NULL;
    }

    char *step4 = replace_placeholder(step3, "{{links}}", links_str);
    free(step3);
    free(links_str);

    return step4;
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
 * Link resolution
 * ============================================================================ */

static int resolve_link(const char *label, const char *folder_path,
                        const char *note_lang, NoteLink *out) {
    struct stat st;
    char target_path[PATH_MAX];

    /* Check if folder exists */
    if (stat(folder_path, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Error: Link target folder '%s' does not exist\n", folder_path);
        return 1;
    }

    /* Try language-specific file first if note has a language */
    int found = 0;
    if (note_lang != NULL) {
        snprintf(target_path, PATH_MAX, "%s/index.%s.md", folder_path, note_lang);
        if (stat(target_path, &st) == 0 && S_ISREG(st.st_mode)) {
            found = 1;
        }
    }

    /* Fallback to index.md */
    if (!found) {
        snprintf(target_path, PATH_MAX, "%s/index.md", folder_path);
        if (stat(target_path, &st) == 0 && S_ISREG(st.st_mode)) {
            found = 1;
        }
    }

    if (!found) {
        if (note_lang != NULL) {
            fprintf(stderr, "Error: No matching note found in '%s' for language '%s'\n",
                    folder_path, note_lang);
        } else {
            fprintf(stderr, "Error: No index.md found in '%s'\n", folder_path);
        }
        return 1;
    }

    /* Parse target file to get title */
    if (init_node_parser() != 0) {
        fprintf(stderr, "Error: Failed to initialize node parser\n");
        return 1;
    }

    Node node = {0};
    parse_node(&node, target_path);
    cleanup_node_parser();

    if (node.title == NULL) {
        fprintf(stderr, "Error: Cannot extract title from '%s'\n", target_path);
        free(node.path);
        return 1;
    }

    /* Populate output with relative path (leading slash) */
    out->label = strdup(label);
    size_t path_len = strlen(target_path) + 2;  /* +1 for '/' +1 for '\0' */
    out->path = malloc(path_len);
    if (out->path != NULL) {
        snprintf(out->path, path_len, "/%s", target_path);
    }
    out->title = node.title;  /* Take ownership */

    free(node.path);

    if (out->label == NULL || out->path == NULL) {
        free(out->label);
        free(out->path);
        free(out->title);
        return 1;
    }

    return 0;
}

/* ============================================================================
 * Note building
 * ============================================================================ */

static int copy_link(NoteLink *dst, NoteLink *src) {
    dst->label = strdup(src->label);
    dst->path = strdup(src->path);
    dst->title = strdup(src->title);

    if (dst->label == NULL || dst->path == NULL || dst->title == NULL) {
        free(dst->label);
        free(dst->path);
        free(dst->title);
        return 1;
    }
    return 0;
}

static int add_note(NoteList *list, const char *lang, const char *title,
                    NoteLinkList *links) {
    if (list->count >= NOTE_MAX_ITEMS) {
        fprintf(stderr, "Error: Too many notes (max %d)\n", NOTE_MAX_ITEMS);
        return 1;
    }

    Note *note = &list->items[list->count];
    note->lang = lang ? strdup(lang) : NULL;
    note->title = strdup(title);
    note->slug = slugify(title);
    note->id = uuid(7);
    note->links.count = 0;

    if (note->title == NULL || note->slug == NULL || note->id == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(note->lang);
        free(note->title);
        free(note->slug);
        free(note->id);
        return 1;
    }

    /* Copy links if provided */
    if (links != NULL) {
        for (int i = 0; i < links->count; i++) {
            if (copy_link(&note->links.items[i], &links->items[i]) != 0) {
                free(note->lang);
                free(note->title);
                free(note->slug);
                free(note->id);
                free_note_link_list(&note->links);
                return 1;
            }
            note->links.count++;
        }
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
    char *link_args[NOTE_MAX_ITEMS];    /* raw "LABEL:PATH" strings */
    int link_count;
    int show_help;
} ParsedArgs;

static int parse_args(int argc, char **argv, ParsedArgs *args) {
    memset(args, 0, sizeof(ParsedArgs));

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            args->show_help = 1;
            return 0;
        }
        else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--link") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -l/--link requires LABEL:PATH argument\n");
                return 1;
            }
            if (args->link_count >= NOTE_MAX_ITEMS) {
                fprintf(stderr, "Error: Too many -l/--link arguments\n");
                return 1;
            }
            args->link_args[args->link_count++] = argv[++i];
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

static int parse_label_path(const char *str, char **label_out, char **path_out) {
    const char *colon = strchr(str, ':');
    if (colon == NULL) {
        fprintf(stderr, "Error: Invalid format '%s', expected LABEL:PATH\n", str);
        return 1;
    }

    size_t label_len = colon - str;
    if (label_len == 0) {
        fprintf(stderr, "Error: Empty label in '%s'\n", str);
        return 1;
    }

    const char *path = colon + 1;
    if (*path == '\0') {
        fprintf(stderr, "Error: Empty path in '%s'\n", str);
        return 1;
    }

    *label_out = strndup(str, label_len);
    *path_out = strdup(path);

    if (*label_out == NULL || *path_out == NULL) {
        free(*label_out);
        free(*path_out);
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
        /* Resolve links for this note (no language) */
        NoteLinkList resolved_links = {0};
        for (int i = 0; i < args.link_count; i++) {
            char *label = NULL;
            char *path = NULL;
            if (parse_label_path(args.link_args[i], &label, &path) != 0) {
                free_note_link_list(&resolved_links);
                cleanup(template, &folder, &notes);
                return 1;
            }
            int err = resolve_link(label, path, NULL, &resolved_links.items[resolved_links.count]);
            free(label);
            free(path);
            if (err != 0) {
                free_note_link_list(&resolved_links);
                cleanup(template, &folder, &notes);
                return 1;
            }
            resolved_links.count++;
        }

        if (add_note(&notes, NULL, args.folder_name, &resolved_links) != 0) {
            free_note_link_list(&resolved_links);
            cleanup(template, &folder, &notes);
            return 1;
        }
        free_note_link_list(&resolved_links);
    } else {
        /* Multiple notes from -n flags */
        for (int i = 0; i < args.lang_title_count; i++) {
            char *lang = NULL;
            char *title = NULL;
            if (parse_lang_title(args.lang_titles[i], &lang, &title) != 0) {
                cleanup(template, &folder, &notes);
                return 1;
            }

            /* Resolve links for this note's language */
            NoteLinkList resolved_links = {0};
            for (int j = 0; j < args.link_count; j++) {
                char *label = NULL;
                char *path = NULL;
                if (parse_label_path(args.link_args[j], &label, &path) != 0) {
                    free(lang);
                    free(title);
                    free_note_link_list(&resolved_links);
                    cleanup(template, &folder, &notes);
                    return 1;
                }
                int err = resolve_link(label, path, lang, &resolved_links.items[resolved_links.count]);
                free(label);
                free(path);
                if (err != 0) {
                    free(lang);
                    free(title);
                    free_note_link_list(&resolved_links);
                    cleanup(template, &folder, &notes);
                    return 1;
                }
                resolved_links.count++;
            }

            int err = add_note(&notes, lang, title, &resolved_links);
            free(lang);
            free(title);
            free_note_link_list(&resolved_links);
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
