# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**jig** is a knowledge graph management tool written in C that follows the Unix philosophy. It provides composable command-line tools that work together via pipes to organize and analyze multi-lingual content structured as a directed graph.

The project emphasizes:
- Small, focused programs that do one thing well
- Text stream processing for universal interfaces
- Composability through standard Unix pipes
- CSV-based data flow between commands

## Build System

Build the project using Make:
```bash
make           # Build the project
make clean     # Clean build artifacts
make rebuild   # Clean and rebuild
```

**Directory Structure:**
- `src/` - C source files, organized by module
- `build/` - Compiled object files (generated)
- `bin/` - Final executable `jig` (generated)
- `datasets/` - Test data for development
- `playground/` - Experimental code

**Manual Compilation:**
```bash
gcc -Wall -Wextra -Werror -I./src \
  src/main.c \
  src/filter/filter.c \
  src/find/find.c \
  src/nodes/nodes.c \
  src/edges/edges.c \
  src/tree/tree.c \
  -o bin/jig
```

**Running the Executable:**
The executable is located at `bin/jig` and can be run directly or added to PATH for global access.

## Architecture

### Module Organization

The codebase follows a modular architecture where each command is a self-contained module:

```
src/
├── main.c              # CLI entry point and command routing
├── filter/             # Filter valid note files by frontmatter
├── find/               # Recursively find files in directory tree
├── nodes/              # Extract node information from files
├── edges/              # Build edges (relationships) between nodes
└── tree/               # Display hierarchical tree visualization
```

Each module contains:
- `<module>.c` - Core implementation
- `<module>.h` - Public interface
- `<module>-cli.c` - CLI argument parsing (if applicable)
- `README.md` - Man page style documentation
- `Makefile` - Standalone build configuration for testing

### Command Pipeline

Commands are designed to work together in pipelines:

```bash
# Typical usage pattern for visualization
jig find . -p '\.md$' | jig filter | jig tree

# Alternative: extract data for analysis
jig find . -p '\.md$' | jig filter | jig nodes | jig edges
```

**Data flow for visualization:**
1. `find` - Outputs file paths (one per line)
2. `filter` - Validates files, outputs valid paths
3. `tree` - Parses files, builds relationships internally, visualizes hierarchy as ASCII tree

**Data flow for analysis:**
1. `find` - Outputs file paths (one per line)
2. `filter` - Validates files, outputs valid paths
3. `nodes` - Parses files, outputs node CSV
4. `edges` - Builds relationships, outputs edge CSV

### Module Details

**1. main.c** - CLI entry point
- Parses first argument to determine command
- Routes to appropriate module function
- Displays help message for unknown commands

**2. filter** - File validation (`src/filter/`)
- Reads file paths from stdin or arguments
- Validates YAML frontmatter structure
- Requires `id` (max 36 bytes) and `title` fields
- Outputs only valid file paths

**3. find** - File discovery (`src/find/`)
- Recursively traverses directories (max 100 levels)
- Skips hidden files (starting with `.`)
- Skips symbolic links
- Optional regex pattern filtering with `-p/--pattern`
- Outputs full file paths

**4. nodes** - Node extraction (`src/nodes/`)
- Reads file paths from stdin
- Parses YAML frontmatter for: `id`, `title`
- Extracts parent link from markdown: `[text](path?label=parent)`
- Outputs CSV: `id,title,path,link`
- Uses regex patterns initialized with `init_node_parser()`

**5. edges** - Relationship building (`src/edges/`)
- Reads node CSV from stdin
- Matches parent links to build relationships
- Outputs CSV: `src_id,src_title,dst_id,dst_title,label,src_path,dst_path`
- Edge label is typically "parent"

**6. tree** - Visualization (`src/tree/`)
- Reads file paths from stdin
- Parses files to extract nodes (using `build_nodes_from_stdin()`)
- Builds edges internally from parent links (using `build_edges_from_nodes()`)
- Renders ASCII tree with Unicode box-drawing characters
- Displays parent-child relationships
- Optional markdown format with `-f md` flag

### Data Structures

**Node (nodes/nodes.h):**
```c
typedef struct {
    char id[37];      // UUID or unique identifier
    char *path;       // File path
    char *link;       // Parent link path
    char *title;      // Node title
} Node;

typedef struct {
    Node *items;
    int count;
} NodeList;
```

**Edge structures** and **Tree structures** follow similar patterns.

## Note File Format

Valid note files must contain YAML frontmatter:

```markdown
---
id: abc123
title: My Note Title
---

Content here.

Optional parent link: [Parent Note](../parent.md?label=parent)
```

**Requirements:**
- `id`: Unique identifier (max 36 bytes, typically UUID)
- `title`: Human-readable title
- Parent link (optional): Format `[text](path?label=parent)`

## Testing Module Code

Each module can be compiled standalone for testing using its local Makefile:

```bash
# Test individual modules
cd src/filter && make
cd src/find && make
cd src/nodes && make

# Run standalone
./bin/<module>-cli <args>
```

Module Makefiles allow testing components in isolation without rebuilding the entire project.

## Development Tools

**Performance measurement:**
```bash
hyperfine "rg -l -i pattern" "find . | jig filter | grep pattern"
```

**Memory leak detection:**
```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes bin/jig <command>
```

## Dependencies

The project uses only standard C libraries:
- **stdio.h** - Standard I/O
- **stdlib.h** - Memory allocation
- **string.h** - String operations
- **regex.h** - POSIX regex for pattern matching
- **dirent.h** - Directory traversal
- **sys/stat.h** - File metadata

No third-party libraries required (previously used cJSON, now removed).

## Key Implementation Details

**Path handling:**
- Uses `PATH_MAX` from `limits.h` for buffer sizes
- Handles paths with/without trailing slashes correctly

**Pattern matching:**
- POSIX Extended Regular Expressions (ERE)
- Case-insensitive matching where appropriate
- Compiled regex patterns cached in node parser

**Memory management:**
- Dynamic allocation for variable-length data (titles, paths, links)
- Explicit cleanup functions for each data structure
- `init_*()` and `cleanup_*()` patterns for resource management

**Error handling:**
- Exit status 0 for success
- Exit status 1 for errors
- Silent filtering (invalid files skipped without error messages)

**File permissions:**
- Standard Unix file operations
- Read-only access to input files
- No file modification or creation

## Unix Philosophy in Practice

Commands follow these principles:

1. **Do one thing well**: Each command has a single, focused purpose
2. **Text streams**: All data flows through stdin/stdout as text
3. **Composability**: Commands combine via pipes for complex operations
4. **Silence is golden**: Output only essential data, no status messages
5. **Exit status**: 0 for success, non-zero for failure

Example workflows:
```bash
# Count total nodes
jig find . | jig filter | jig nodes | tail -n +2 | wc -l

# Extract node titles
jig find . | jig filter | jig nodes | tail -n +2 | cut -d, -f2

# Find orphan nodes (no parent)
jig find . | jig filter | jig nodes | tail -n +2 | awk -F, '$4 == ""'

# Visualize full hierarchy
jig find . -p '\.md$' | jig filter | jig tree
```

## Compilation Flags

The project uses strict compiler flags:
- `-Wall` - Enable all warnings
- `-Wextra` - Enable extra warnings
- `-Werror` - Treat warnings as errors
- `-I./src` - Include header search path

This ensures code quality and catches common mistakes at compile time.
