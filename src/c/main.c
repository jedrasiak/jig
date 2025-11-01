#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vertex.h"
#include "search.h"

char *VERSION = "0.1.0";

int cmd_search(int argc, char *argv[]);
int cmd_nv(int argc, char *argv[]);
int print_help();
int print_version();
int print_usage();

int main(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "+vh")) != -1) {
        switch (opt) {
            case 'v':
                print_version();
                exit(EXIT_SUCCESS);
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);
            case '?':
                fprintf(stderr, "Error: unknown global option -%c\n", optopt);
                fprintf(stderr, "Try '%s -h' for help\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        // No subcommand provided
        fprintf(stderr, "Error: No subcommand specified\n");
        print_usage();
        exit(EXIT_FAILURE);
    }

    // parse subcommand
    const char *subcommand = argv[optind];

    if (strcmp(subcommand, "search") == 0) {
        cmd_search(argc, argv);
    } else if (strcmp(subcommand, "nv") == 0) {
        cmd_nv(argc, argv);
    } else {
        fprintf(stderr, "Error: Unknown subcommand '%s'\n", subcommand);
        print_usage();
        exit(EXIT_FAILURE);
    }
    
    return 0;
}

int cmd_search(int argc, char *argv[]) {
    int opt;
    const char *usage = "\nUsage: scientia search [OPTIONS] <query> \n"
                        "Options:\n"
                        "  -p <path>          Path to start the search from (default: current directory)\n"
                        "  -a <algorithm>     Search algorithm to use (e.g., re)\n"
                        "  -o <output>        Output format (e.g., stdout, out.csv)\n";

    char *path = ".";
    char *algorithm = NULL;
    char *output = NULL;
    char *query = NULL;
    
    optind++; // Move to the next argument after subcommand

    // Parse options for the 'search' subcommand
    while ((opt = getopt(argc, argv, "+:p:a:o:")) != -1) {
        switch (opt) {
            case 'p':
                path = optarg;
                break;
            case 'a':
                algorithm = optarg;
                break;
            case 'o':
                output = optarg;
                break;
            case ':':
                fprintf(stderr, "Error: -%c requires an argument\n", optopt);
                fprintf(stderr, "%s\n", usage);
                exit(EXIT_FAILURE);
            case '?':
                fprintf(stderr, "Error: unknown option -%c\n", optopt);
                fprintf(stderr, "%s\n", usage);
                exit(EXIT_FAILURE);
        }
    }
    
    // After getopt finishes, optind points to first non-option argument
    // This should be the query
    if (optind < argc) {
        query = argv[optind];
    }
    
    // Validate required arguments
    if (!query) {
        fprintf(stderr, "Error: <query> is required\n");
        fprintf(stderr, "%s\n", usage);
        exit(EXIT_FAILURE);
    }
    
    if (!algorithm) {
        fprintf(stderr, "Error: -a <algorithm> is required\n");
        fprintf(stderr, "%s\n", usage);
        exit(EXIT_FAILURE);
    }
    
    if (!output) {
        fprintf(stderr, "Error: -o <output> is required\n");
        fprintf(stderr, "%s\n", usage);
        exit(EXIT_FAILURE);
    }

    int result = search(path, algorithm, output, query);

    return result;
}

int cmd_nv(int argc, char *argv[]) {
    char *vertex_name = NULL;
    
    // Skip the subcommand name
    optind++;
    
    // Check if there's a positional argument
    if (optind < argc) {
        vertex_name = argv[optind];
    }

    int result = nv(vertex_name);

    return result;
}

int print_help() {
    printf("Usage:\n");
    printf("  scientia [OPTIONS] <command> [COMMAND_OPTIONS]\n");
    printf("\n");
    printf("Global Options:\n");
    printf("  -h               Show this help message and exit\n");
    printf("  -v               Show version information\n");
    printf("\n");
    printf("Commands:\n");
    printf("  nv [path]        Create a new vertex (optional: at specified path, default: current dir)\n");
    printf("  search           Search for a query in the knowledge graph\n");
    printf("\n");
    printf("Search Command:\n");
    printf("  Usage: scientia search [OPTIONS] <query>\n");
    printf("  Options:\n");
    printf("    -p <path>      Path to start the search from (default: current directory)\n");
    printf("    -a <algorithm> Search algorithm to use (required, e.g., 're' for regex)\n");
    printf("    -o <output>    Output format (required, e.g., 'stdout', 'out.csv')\n");
    return 0;
}

int print_version() {
    printf("%s\n", VERSION);
    return 0;
}

int print_usage() {
    printf("Usage: scientia <command> [OPTIONS]\n");
    printf("Try 'scientia -h' for more information.\n");
    return 0;
}