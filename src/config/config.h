#ifndef CONFIG_H
#define CONFIG_H

#define MAX_KEY 64
#define MAX_ENDPOINT 128
#define MAX_LINE 512

typedef struct {
    char key[MAX_KEY];
    char endpoint[MAX_ENDPOINT];
} Provider;

int config(int argc, char **argv);

#endif /* CONFIG_H */