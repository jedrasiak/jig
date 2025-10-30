# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Scientia is a knowledge graph management tool written in C. It organizes multi-lingual content as a directed graph where vertices (directories) contain markdown files for each supported language. The project uses a graph.json configuration file at the root to define available languages.

## Build System

Build the project using Make:
```bash
make           # Build the project
make clean     # Clean build artifacts
make rebuild   # Clean and rebuild
make info      # Show build information
```

**Directory Structure:**
- `src/c/` - C source files
- `include/` - Header files
- `lib/` - Third-party libraries (cJSON)
- `build/` - Compiled object files (generated)
- `bin/` - Final executable (generated, added to PATH for global access)

**Manual Compilation:**
```bash
gcc -Wall -Wextra -I./include src/c/main.c src/c/vertex.c src/c/graph.c lib/cJSON.c -o bin/scientia
```

**Running the Executable:**
The `bin/` directory is added to PATH, so the `scientia` command can be executed from anywhere without needing to reference the relative path.

## Architecture

### Module Organization

The codebase is organized into three main modules:

1. **main.c** - CLI entry point and command routing
   - Handles command-line argument parsing
   - Routes commands to appropriate modules (nv, ne)

2. **vertex.c/h** - Vertex (content node) management
   - Creates vertex directories with multi-lingual markdown files
   - Uses graph configuration to determine which language files to create
   - Each vertex contains `index.<lang>.md` files with frontmatter (title, slug)

3. **graph.c/h** - Graph configuration and navigation
   - `is_root()` - Checks if a directory contains graph.json
   - `find_root()` - Traverses upward from CWD to find graph root
   - `get_config()` - Parses graph.json using cJSON to extract language settings
   - `free_config()` - Manages memory cleanup for GraphConfig structures

### Graph Configuration

The graph.json file at the graph root defines:
```json
{
    "languages": {
        "current": "en",
        "available": ["en", "pl", "de"]
    }
}
```

All operations assume the current working directory is within a scientia graph (contains or is within a directory with graph.json).

### Vertex Structure

A vertex is a directory containing markdown files for each configured language:
```
note-1/
├── index.en.md
├── index.pl.md
└── index.de.md
```

Each markdown file has frontmatter:
```markdown
---
title: note-1
slug: note-1
---
```

Vertices are created relative to the current working directory, not the graph root.

## Testing Module Code

Individual modules can be compiled with TEST flag for standalone testing. Example from graph.c:146:
```bash
gcc -DTEST -I./include src/c/graph.c lib/cJSON.c -o bin/test_graph
./bin/test_graph
```

This pattern allows testing graph utilities (is_root, find_root, get_config) independently.

## Dependencies

- **cJSON** (lib/cJSON.c, include/cJSON.h) - JSON parsing library for graph.json configuration

## Key Implementation Details

- Path handling uses PATH_MAX from limits.h for buffer sizes
- Graph root discovery uses upward directory traversal with dirname() from libgen.h
- Vertex creation dynamically uses language configuration from graph.json
- Memory management: GraphConfig structures must be freed with free_config() after use
- The project uses standard Unix APIs (stat, mkdir) with 0755 permissions for directories
