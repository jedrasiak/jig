#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"

static void help(void);
static void print(Settings *settings);
static char *trim(char *str);

int config(int argc, char **argv) {
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        }
    }

    // Default: parse config file
    Settings settings = load_config();
    print(&settings);
    free_settings(&settings);
    return 0;
}

/* --- */
static void help(void) {
    printf("Usage jig config [OPTIONS]\n");
    printf("\n");
}

static char *trim(char *str) {
    // Trim leading whitespace
    while (*str == ' ' || *str == '\t') str++;

    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t')) {
        *end = '\0';
        end--;
    }

    return str;
}

Settings load_config(void) {
    Settings settings = {0};
    settings.providers.items = malloc(sizeof(Provider) * MAX_PROVIDERS);
    settings.providers.count = 0;

    FILE *fptr = fopen("./jig.conf", "r");
    if (fptr == NULL) {
        fprintf(stderr, "Error: Unable to open config file.\n");
        fprintf(stderr, "Ensure you are in the project root directory and run 'jig init'.\n");
        free(settings.providers.items);
        settings.providers.items = NULL;
        return settings;
    }

    char line[MAX_LINE];
    char section[MAX_KEY] = "";
    Provider *current_provider = NULL;

    while (fgets(line, sizeof(line), fptr)) {
        // Trim newline
        line[strcspn(line, "\n")] = 0;

        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\0') {
            continue;
        }

        // Check for section header
        if (line[0] == '[') {
            char *end = strchr(line, ']');
            if (end) {
                size_t len = end - line - 1;
                strncpy(section, line + 1, len);
                section[len] = '\0';
            }

            // Check if this is a provider section (e.g., [provider.mistral])
            if (strncmp(section, "provider.", 9) == 0) {
                if (settings.providers.count < MAX_PROVIDERS) {
                    current_provider = &settings.providers.items[settings.providers.count];
                    memset(current_provider, 0, sizeof(Provider));
                    strncpy(current_provider->name, section + 9, MAX_NAME - 1);
                    settings.providers.count++;
                }
            } else {
                current_provider = NULL;
            }
            continue;
        }

        // Parse key=value
        char *equals = strchr(line, '=');
        if (equals) {
            *equals = '\0';
            char *key = trim(line);
            char *value = trim(equals + 1);

            // Store values in current provider
            if (current_provider != NULL) {
                if (strcmp(key, "key") == 0) {
                    strncpy(current_provider->key, value, MAX_KEY - 1);
                } else if (strcmp(key, "endpoint") == 0) {
                    strncpy(current_provider->endpoint, value, MAX_ENDPOINT - 1);
                }
            }
        }
    }

    fclose(fptr);
    return settings;
}

static void print(Settings *settings) {
    for (int i = 0; i < settings->providers.count; i++) {
        Provider *p = &settings->providers.items[i];
        printf("[provider.%s]\n", p->name);
        printf("key:      %s\n", p->key);
        printf("endpoint: %s\n", p->endpoint);
    }
}

void free_settings(Settings *settings) {
    if (settings->providers.items) {
        free(settings->providers.items);
        settings->providers.items = NULL;
    }
    settings->providers.count = 0;
}

Provider *find_provider(Settings *settings, const char *name) {
    for (int i = 0; i < settings->providers.count; i++) {
        if (strcmp(settings->providers.items[i].name, name) == 0) {
            return &settings->providers.items[i];
        }
    }
    return NULL;
}
