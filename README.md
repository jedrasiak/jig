# jig

A knowledge graph management tool written in C that follows the Unix philosophy. Provides composable command-line tools that work together via pipes to organize and analyze multi-lingual content structured as a directed graph.

## Unix Philosophy

**jig** embodies the core Unix principles:

- **Do one thing well** - Each command has a single, focused purpose
- **Work together** - Commands combine via pipes for complex operations
- **Handle text streams** - Universal interface using CSV and newline-delimited text
- **Silence is golden** - Output only essential data, no status messages
- **Exit status matters** - 0 for success, non-zero for failure

This design enables powerful workflows by chaining simple tools together.

## Quick Start

```bash
# Build the project
make

# Visualize your note hierarchy
jig find . -p '\.md$' | jig filter | jig nodes | jig edges | jig tree
```

## Commands

**jig** provides these composable commands:

- **jig-find** - Recursively find files in directory tree
- **jig-filter** - Validate note files by frontmatter criteria
- **jig-nodes** - Extract node information from files
- **jig-edges** - Build relationships between nodes
- **jig-tree** - Display hierarchical tree visualization

Each command can be invoked as `jig <command>` (subcommand mode) or `jig-<command>` (standalone mode).

## Usage

### Basic Workflow

The typical workflow chains commands together:

```bash
# Complete pipeline: find → filter → nodes → edges → tree
jig find . -p '\.md$' | jig filter | jig nodes | jig edges | jig tree
```

Output:
```
Project Overview
├── Task List
│   ├── Feature A
│   └── Feature B
└── Documentation
    └── API Reference
```

### Finding Files

```bash
# Find all markdown files
jig find . -p '\.md$'

# Find files in specific directory
jig find ./notes -p '\.md$'
```

### Filtering Valid Notes

```bash
# Filter files with valid frontmatter
jig find . -p '\.md$' | jig filter
```

Valid notes must have YAML frontmatter with `id` and `title`:
```markdown
---
id: abc123
title: My Note
---

Content here.

[Parent Note](../parent.md?label=parent)
```

### Extracting Nodes

```bash
# Extract node information as CSV
jig find . | jig filter | jig nodes

# Output:
# id,title,path,link
# abc123,Project Overview,./notes/project.md,
# def456,Task List,./notes/tasks.md,./notes/project.md
```

### Building Edges

```bash
# Build parent-child relationships
jig find . | jig filter | jig nodes | jig edges

# Output:
# src_id,src_title,dst_id,dst_title,label,src_path,dst_path
# def456,Task List,abc123,Project Overview,parent,./notes/tasks.md,./notes/project.md
```

### Analysis Examples

```bash
# Count total nodes
jig find . | jig filter | jig nodes | tail -n +2 | wc -l

# Extract only titles
jig find . | jig filter | jig nodes | tail -n +2 | cut -d, -f2

# Find orphan nodes (no parent)
jig find . | jig filter | jig nodes | tail -n +2 | awk -F, '$4 == ""'

# Count relationships
jig find . | jig filter | jig nodes | jig edges | tail -n +2 | wc -l

# Export tree structure
jig find ./notes | jig filter | jig nodes | jig edges | jig tree > hierarchy.txt
```

## Installation

### Building from Source

```bash
# Build the project
make

# Clean build artifacts
make clean

# Rebuild from scratch
make rebuild
```

The executable will be created at `bin/jig`.

### Manual Compilation

If you prefer to compile without Make:

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

### Adding to PATH

To use `jig` globally:

```bash
# Add to your shell profile (~/.bashrc, ~/.zshrc, etc.)
export PATH="$PATH:/path/to/jig/bin"
```

## Documentation

Each command has detailed documentation in man page format:

- [jig-find(1)](src/find/README.md) - Find files recursively
- [jig-filter(1)](src/filter/README.md) - Filter valid notes
- [jig-nodes(1)](src/nodes/README.md) - Extract nodes
- [jig-edges(1)](src/edges/README.md) - Build edges
- [jig-tree(1)](src/tree/README.md) - Visualize hierarchy

## Contributing

### Project Structure

```
jig/
├── src/                # Source code organized by module
│   ├── main.c          # CLI entry point and command routing
│   ├── filter/         # Filter module
│   ├── find/           # Find module
│   ├── nodes/          # Nodes module
│   ├── edges/          # Edges module
│   └── tree/           # Tree module
├── build/              # Compiled object files (generated)
├── bin/                # Final executable (generated)
├── datasets/           # Test data for development
└── Makefile            # Build configuration
```

### Building

**Standard build:**
```bash
make
```

**Testing individual modules:**

Each module has a standalone Makefile for isolated testing:

```bash
cd src/filter && make
cd src/find && make
cd src/nodes && make

# Run standalone
./bin/<module>-cli <args>
```

### Compilation

The project uses strict compiler flags to ensure code quality:

```bash
gcc -Wall -Wextra -Werror -I./src <sources> -o bin/jig
```

**Flags:**
- `-Wall` - Enable all warnings
- `-Wextra` - Enable extra warnings
- `-Werror` - Treat warnings as errors
- `-I./src` - Include header search path

**Dependencies:**

Only standard C libraries (no third-party dependencies):
- `stdio.h` - Standard I/O
- `stdlib.h` - Memory allocation
- `string.h` - String operations
- `regex.h` - POSIX regex
- `dirent.h` - Directory traversal
- `sys/stat.h` - File metadata

### Performance Measuring

Use [hyperfine](https://github.com/sharkdp/hyperfine) to benchmark commands:

```bash
# Compare with standard tools
hyperfine "rg -l -i pattern" "find . | jig filter | grep pattern"

# Measure pipeline performance
hyperfine "jig find . -p '\.md$' | jig filter | jig nodes"

# Compare different approaches
hyperfine "jig find . -p '\.md$'" "find . -name '*.md'"
```

### Memory Error Detection

Use [Valgrind](https://valgrind.org/) to detect memory leaks and errors:

```bash
# Check for memory leaks
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes bin/jig <command>

# Example: Check filter command
valgrind --leak-check=full bin/jig filter notes/test.md

# Full pipeline check
find . -name "*.md" | valgrind --leak-check=full bin/jig filter
```

**Expected output:**
```
HEAP SUMMARY:
    in use at exit: 0 bytes in 0 blocks
  total heap usage: X allocs, X frees, Y bytes allocated

All heap blocks were freed -- no leaks are possible
```

### Code Guidelines

- Follow Unix philosophy: small, focused, composable tools
- Use only standard C libraries
- All dynamic memory must be properly freed
- Implement `init_*()` and `cleanup_*()` patterns for resources
- Handle errors gracefully with appropriate exit codes
- Output only essential data to stdout
- Use stderr for error messages (if needed)
- Write man page style documentation for new commands

## License

See LICENSE file for details.
