#ifndef CONFIG_H
#define CONFIG_H

#define MAX_NAME 64
#define MAX_KEY 64
#define MAX_ENDPOINT 128
#define MAX_LINE 512
#define MAX_PROVIDERS 16

typedef struct {
    char name[MAX_NAME];
    char key[MAX_KEY];
    char endpoint[MAX_ENDPOINT];
} Provider;

typedef struct {
    Provider *items;
    int count;
} ProviderList;

typedef struct {
    ProviderList providers;
} Settings;

int config(int argc, char **argv);

Settings load_config(void);
void free_settings(Settings *settings);
Provider *find_provider(Settings *settings, const char *name);

#endif /* CONFIG_H */