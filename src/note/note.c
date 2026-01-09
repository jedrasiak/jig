#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include "note.h"
#include "../uuid/uuid.h"
#include "../slugify/slugify.h"

#define MAX_NAMES 64
#define MAX_TEMPLATE_SIZE 10485760  /* 10MB */

/**
 * Language-title pair for multilingual notes
 */
typedef struct {
    char *lang;
    char *title;
} LangTitle;

/**
 * Display help message following Unix conventions
 */
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

/**
 * Get default template content
 * Returns allocated string that caller must free
 */
static char *get_default_template(void) {
    const char *template = "---\nid: {{id}}\n---\n\n# {{title}}\n";
    char *result = strdup(template);
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
    }
    return result;
}

/**
 * Replace all occurrences of placeholder with value in template
 * Returns newly allocated string that caller must free
 * If placeholder not found, returns copy of template
 */
static char *replace_placeholder(const char *template, const char *placeholder, const char *value) {
    if (template == NULL || placeholder == NULL || value == NULL) {
        return NULL;
    }

    /* Find first occurrence */
    const char *pos = strstr(template, placeholder);

    /* If placeholder not found, return copy of template */
    if (pos == NULL) {
        return strdup(template);
    }

    /* Calculate sizes */
    size_t template_len = strlen(template);
    size_t placeholder_len = strlen(placeholder);
    size_t value_len = strlen(value);

    /* Count occurrences to calculate final size */
    int count = 0;
    const char *tmp = template;
    while ((tmp = strstr(tmp, placeholder)) != NULL) {
        count++;
        tmp += placeholder_len;
    }

    /* Calculate new size */
    size_t new_size = template_len - (count * placeholder_len) + (count * value_len) + 1;

    /* Allocate new buffer */
    char *result = malloc(new_size);
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    /* Build result by replacing all occurrences */
    char *dst = result;
    const char *src = template;

    while ((pos = strstr(src, placeholder)) != NULL) {
        /* Copy before placeholder */
        size_t before_len = pos - src;
        memcpy(dst, src, before_len);
        dst += before_len;

        /* Copy value */
        memcpy(dst, value, value_len);
        dst += value_len;

        /* Move source past placeholder */
        src = pos + placeholder_len;
    }

    /* Copy remaining */
    strcpy(dst, src);

    return result;
}

/**
 * Read template file content
 * Returns allocated string that caller must free
 */
static char *read_template_file(const char *filepath) {
    FILE *fptr;
    long filesize;
    char *content;

    /* Open file */
    fptr = fopen(filepath, "r");
    if (fptr == NULL) {
        fprintf(stderr, "Error: Cannot open template file '%s'\n", filepath);
        return NULL;
    }

    /* Get file size */
    fseek(fptr, 0L, SEEK_END);
    filesize = ftell(fptr);
    fseek(fptr, 0L, SEEK_SET);

    /* Check size limit */
    if (filesize > MAX_TEMPLATE_SIZE) {
        fprintf(stderr, "Error: Template file too large (max 10MB)\n");
        fclose(fptr);
        return NULL;
    }

    /* Allocate buffer */
    content = malloc(filesize + 1);
    if (content == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(fptr);
        return NULL;
    }

    /* Read content */
    for (long i = 0; i < filesize; i++) {
        content[i] = fgetc(fptr);
    }
    content[filesize] = '\0';

    fclose(fptr);
    return content;
}

/**
 * Parse LANG:TITLE string into LangTitle struct
 * Returns 0 on success, 1 on error
 */
static int parse_lang_title(const char *str, LangTitle *out) {
    if (str == NULL || out == NULL) {
        return 1;
    }

    /* Find first colon */
    const char *colon = strchr(str, ':');
    if (colon == NULL) {
        fprintf(stderr, "Error: Invalid format '%s', expected LANG:TITLE\n", str);
        return 1;
    }

    /* Extract language (before colon) */
    size_t lang_len = colon - str;
    if (lang_len == 0) {
        fprintf(stderr, "Error: Empty language in '%s'\n", str);
        return 1;
    }

    out->lang = malloc(lang_len + 1);
    if (out->lang == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }
    memcpy(out->lang, str, lang_len);
    out->lang[lang_len] = '\0';

    /* Extract title (after colon) */
    const char *title_start = colon + 1;
    size_t title_len = strlen(title_start);
    if (title_len == 0) {
        fprintf(stderr, "Error: Empty title in '%s'\n", str);
        free(out->lang);
        out->lang = NULL;
        return 1;
    }

    out->title = malloc(title_len + 1);
    if (out->title == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(out->lang);
        out->lang = NULL;
        return 1;
    }
    memcpy(out->title, title_start, title_len);
    out->title[title_len] = '\0';

    return 0;
}

/**
 * Create file with content
 * Returns 0 on success, 1 on error
 */
static int create_file(const char *filepath, const char *content) {
    FILE *fptr;

    /* Check if file already exists */
    fptr = fopen(filepath, "r");
    if (fptr != NULL) {
        fclose(fptr);
        fprintf(stderr, "Error: File '%s' already exists\n", filepath);
        return 1;
    }

    /* Create file */
    fptr = fopen(filepath, "w");
    if (fptr == NULL) {
        fprintf(stderr, "Error: Cannot create file '%s'\n", filepath);
        return 1;
    }

    /* Write content */
    fprintf(fptr, "%s", content);
    fclose(fptr);

    return 0;
}

/**
 * Free LangTitle array
 */
static void cleanup_names(LangTitle *names, int count) {
    for (int i = 0; i < count; i++) {
        free(names[i].lang);
        free(names[i].title);
    }
}

/**
 * Main entry point for note command
 */
int note(int argc, char **argv) {
    char *folder_name = NULL;
    char *template_path = NULL;
    LangTitle names[MAX_NAMES];
    int name_count = 0;

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        }
        else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--name") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -n/--name requires LANG:TITLE argument\n");
                return 1;
            }
            if (name_count >= MAX_NAMES) {
                fprintf(stderr, "Error: Too many -n/--name arguments (max %d)\n", MAX_NAMES);
                return 1;
            }
            if (parse_lang_title(argv[i + 1], &names[name_count]) != 0) {
                return 1;
            }
            name_count++;
            i++;
        }
        else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--template") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -t/--template requires a file path\n");
                return 1;
            }
            template_path = argv[i + 1];
            i++;
        }
        else if (argv[i][0] != '-') {
            folder_name = argv[i];
        }
        else {
            fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
            help();
            return 1;
        }
    }

    /* Validate folder name */
    if (folder_name == NULL) {
        fprintf(stderr, "Error: Folder name is required\n");
        help();
        cleanup_names(names, name_count);
        return 1;
    }

    /* Generate slug from folder name */
    char *slug = slugify(folder_name);
    if (slug == NULL) {
        cleanup_names(names, name_count);
        return 1;
    }

    /* Check if directory exists */
    struct stat st;
    if (stat(slug, &st) == 0) {
        fprintf(stderr, "Error: Directory '%s' already exists\n", slug);
        free(slug);
        cleanup_names(names, name_count);
        return 1;
    }

    /* Create directory */
    if (mkdir(slug, 0755) != 0) {
        fprintf(stderr, "Error: Failed to create directory '%s'\n", slug);
        free(slug);
        cleanup_names(names, name_count);
        return 1;
    }

    /* Get template content */
    char *template_raw = NULL;
    if (template_path != NULL) {
        template_raw = read_template_file(template_path);
    } else {
        template_raw = get_default_template();
    }
    if (template_raw == NULL) {
        free(slug);
        cleanup_names(names, name_count);
        return 1;
    }

    int result = 0;

    /* Create files */
    if (name_count == 0) {
        /* No -n flags: create single index.md using folder_name as title and slug */
        char *template_with_slug = replace_placeholder(template_raw, "{{slug}}", slug);
        if (template_with_slug == NULL) {
            free(template_raw);
            free(slug);
            return 1;
        }

        char *template_with_title = replace_placeholder(template_with_slug, "{{title}}", folder_name);
        free(template_with_slug);
        if (template_with_title == NULL) {
            free(template_raw);
            free(slug);
            return 1;
        }

        char *uuid_str = uuid(7);
        if (uuid_str == NULL) {
            free(template_with_title);
            free(template_raw);
            free(slug);
            return 1;
        }

        char *final_content = replace_placeholder(template_with_title, "{{id}}", uuid_str);
        free(template_with_title);
        free(uuid_str);
        if (final_content == NULL) {
            free(template_raw);
            free(slug);
            return 1;
        }

        char filepath[PATH_MAX];
        snprintf(filepath, PATH_MAX, "%s/index.md", slug);
        result = create_file(filepath, final_content);
        free(final_content);
    } else {
        /* Create index.LANG.md for each -n entry with its own slug */
        for (int i = 0; i < name_count; i++) {
            char *title_slug = slugify(names[i].title);
            if (title_slug == NULL) {
                result = 1;
                break;
            }

            char *template_with_slug = replace_placeholder(template_raw, "{{slug}}", title_slug);
            free(title_slug);
            if (template_with_slug == NULL) {
                result = 1;
                break;
            }

            char *template_with_title = replace_placeholder(template_with_slug, "{{title}}", names[i].title);
            free(template_with_slug);
            if (template_with_title == NULL) {
                result = 1;
                break;
            }

            char *uuid_str = uuid(7);
            if (uuid_str == NULL) {
                free(template_with_title);
                result = 1;
                break;
            }

            char *final_content = replace_placeholder(template_with_title, "{{id}}", uuid_str);
            free(template_with_title);
            free(uuid_str);
            if (final_content == NULL) {
                result = 1;
                break;
            }

            char filepath[PATH_MAX];
            snprintf(filepath, PATH_MAX, "%s/index.%s.md", slug, names[i].lang);
            if (create_file(filepath, final_content) != 0) {
                free(final_content);
                result = 1;
                break;
            }
            free(final_content);
        }
    }

    /* Cleanup */
    free(slug);
    free(template_raw);
    cleanup_names(names, name_count);

    return result;
}
