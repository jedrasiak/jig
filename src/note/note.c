#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include "note.h"
#include "../uuid/uuid.h"
#include "../slugify/slugify.h"

#define MAX_LANGUAGES 64
#define MAX_TEMPLATE_SIZE 10485760  /* 10MB */

/**
 * Display help message following Unix conventions
 */
static void help(void) {
    printf("Usage: jig note <TITLE> [OPTIONS]\n");
    printf("       jig-note <TITLE> [OPTIONS]\n");
    printf("\n");
    printf("Create note scaffold with directory and markdown file(s).\n");
    printf("\n");
    printf("Arguments:\n");
    printf("  TITLE              Mandatory title for the note\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help         Display this help and exit\n");
    printf("  -l, --lang LANGS   Comma-separated languages (creates index.LANG.md)\n");
    printf("  -t, --template PATH Use custom template file\n");
    printf("\n");
    printf("Template Placeholders:\n");
    printf("  {{id}}             Replaced with generated UUID v7\n");
    printf("  {{title}}          Replaced with provided title\n");
    printf("  {{slug}}           Replaced with slugified title\n");
    printf("\n");
    printf("Default template (if no -t flag):\n");
    printf("  ---\n");
    printf("  id: {{id}}\n");
    printf("  ---\n");
    printf("\n");
    printf("  # {{title}}\n");
    printf("\n");
    printf("Examples:\n");
    printf("  jig note \"My New Note\"\n");
    printf("  jig note \"My Note\" -l \"en,pl\"\n");
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
 * Parse comma-separated language list
 * Stores allocated strings in langs_out array
 * Returns count of languages parsed
 */
static int parse_languages(const char *lang_str, char **langs_out, int max_langs) {
    if (lang_str == NULL || langs_out == NULL) {
        return 0;
    }

    /* Make a copy since strtok_r modifies the string */
    char *copy = strdup(lang_str);
    if (copy == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 0;
    }

    int count = 0;
    char *saveptr;
    char *token = strtok_r(copy, ",", &saveptr);

    while (token != NULL && count < max_langs) {
        /* Trim leading/trailing whitespace */
        while (*token == ' ' || *token == '\t') {
            token++;
        }

        size_t len = strlen(token);
        while (len > 0 && (token[len-1] == ' ' || token[len-1] == '\t')) {
            len--;
        }

        if (len > 0) {
            /* Allocate and copy language string */
            langs_out[count] = malloc(len + 1);
            if (langs_out[count] == NULL) {
                fprintf(stderr, "Error: Memory allocation failed\n");
                free(copy);
                return count;
            }
            memcpy(langs_out[count], token, len);
            langs_out[count][len] = '\0';
            count++;
        }

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(copy);
    return count;
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
 * Free language string array
 */
static void cleanup_languages(char **langs, int count) {
    for (int i = 0; i < count; i++) {
        free(langs[i]);
    }
}

/**
 * Main entry point for note command
 */
int note(int argc, char **argv) {
    char *title = NULL;
    char *lang_str = NULL;
    char *template_path = NULL;
    char *langs[MAX_LANGUAGES];
    int lang_count = 0;
    int result = 0;

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        }
        else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--lang") == 0) {
            if (i + 1 < argc) {
                lang_str = argv[i + 1];
                i++;
            } else {
                fprintf(stderr, "Error: -l/--lang requires a language list\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--template") == 0) {
            if (i + 1 < argc) {
                template_path = argv[i + 1];
                i++;
            } else {
                fprintf(stderr, "Error: -t/--template requires a file path\n");
                return 1;
            }
        }
        else if (argv[i][0] != '-') {
            /* Positional argument is the title */
            title = argv[i];
        }
        else {
            fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
            help();
            return 1;
        }
    }

    /* Validate title */
    if (title == NULL) {
        fprintf(stderr, "Error: TITLE is required\n");
        help();
        return 1;
    }

    /* Generate slug */
    char *slug = slugify(title);
    if (slug == NULL) {
        return 1;
    }

    /* Check if directory exists */
    struct stat st;
    if (stat(slug, &st) == 0) {
        fprintf(stderr, "Error: Directory '%s' already exists\n", slug);
        free(slug);
        return 1;
    }

    /* Create directory */
    if (mkdir(slug, 0755) != 0) {
        fprintf(stderr, "Error: Failed to create directory '%s'\n", slug);
        free(slug);
        return 1;
    }

    /* Get template content */
    char *template_raw = NULL;
    if (template_path != NULL) {
        template_raw = read_template_file(template_path);
        if (template_raw == NULL) {
            free(slug);
            return 1;
        }
    } else {
        template_raw = get_default_template();
        if (template_raw == NULL) {
            free(slug);
            return 1;
        }
    }

    /* Replace {{title}} and {{slug}} placeholders (keep {{id}} for per-file replacement) */
    char *template_with_title = replace_placeholder(template_raw, "{{title}}", title);
    free(template_raw);
    if (template_with_title == NULL) {
        free(slug);
        return 1;
    }

    char *template_base = replace_placeholder(template_with_title, "{{slug}}", slug);
    free(template_with_title);
    if (template_base == NULL) {
        free(slug);
        return 1;
    }

    /* Parse languages if provided */
    if (lang_str != NULL) {
        lang_count = parse_languages(lang_str, langs, MAX_LANGUAGES);
        if (lang_count == 0) {
            fprintf(stderr, "Error: No valid languages parsed\n");
            free(slug);
            free(template_base);
            return 1;
        }
    }

    /* Create files */
    if (lang_count == 0) {
        /* No languages: create single index.md with one UUID */
        char *uuid_str = uuid(7);
        if (uuid_str == NULL) {
            free(slug);
            free(template_base);
            return 1;
        }

        char *final_content = replace_placeholder(template_base, "{{id}}", uuid_str);
        free(uuid_str);
        if (final_content == NULL) {
            free(slug);
            free(template_base);
            return 1;
        }

        char filepath[PATH_MAX];
        snprintf(filepath, PATH_MAX, "%s/index.md", slug);
        result = create_file(filepath, final_content);
        free(final_content);
    } else {
        /* Create index.LANG.md for each language with unique UUID per file */
        for (int i = 0; i < lang_count; i++) {
            /* Generate unique UUID for this file */
            char *uuid_str = uuid(7);
            if (uuid_str == NULL) {
                result = 1;
                break;
            }

            /* Replace {{id}} with this file's UUID */
            char *final_content = replace_placeholder(template_base, "{{id}}", uuid_str);
            free(uuid_str);
            if (final_content == NULL) {
                result = 1;
                break;
            }

            /* Create the file */
            char filepath[PATH_MAX];
            snprintf(filepath, PATH_MAX, "%s/index.%s.md", slug, langs[i]);
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
    free(template_base);
    cleanup_languages(langs, lang_count);

    return result;
}
