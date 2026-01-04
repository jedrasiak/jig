# jig-tree(1)

## NAME

**jig-tree** - display hierarchical tree visualization of node relationships

## SYNOPSIS

```
jig tree [OPTIONS]
jig-tree [OPTIONS]
```

## DESCRIPTION

**jig-tree** reads edge CSV data from stdin, builds a hierarchical tree structure from the relationships, and renders an ASCII tree visualization using Unicode box-drawing characters.

The command processes parent-child relationships between nodes and displays them as an indented tree, making it easy to visualize the structure of your knowledge graph or note hierarchy.

Nodes without parents are displayed as root nodes. Each node shows its title, and child nodes are displayed with proper indentation and tree branch characters.

## OPTIONS

- **-h, --help** - Display help information and exit

## INPUT FORMAT

Expects CSV from stdin with columns: src_id,src_title,dst_id,dst_title,label,src_path,dst_path
(This is the output format from jig-edges)

The tree builder processes edges where the label is "parent" to construct the hierarchy.

## OUTPUT FORMAT

ASCII tree using Unicode box-drawing characters:
- `├──` for intermediate children
- `└──` for last children
- `│   ` for vertical connectors
- `    ` for spacing

Each node is displayed with its title, and the tree structure shows parent-child relationships visually.

## EXIT STATUS

- **0** - Success (tree rendered successfully)
- **1** - Error (memory allocation failure or invalid input)

## EXAMPLES

Visualize the complete note hierarchy:
```bash
$ jig find . -p '\.md$' | jig filter | jig nodes | jig edges | jig tree
Project Overview
├── Task List
│   ├── Feature A
│   └── Feature B
└── Documentation
    └── API Reference
```

Use with specific directories:
```bash
# Visualize only notes in a specific directory
$ jig find ./notes -p '\.md$' | jig filter | jig nodes | jig edges | jig tree

# Combine with grep to filter specific branches
$ jig find . | jig filter | jig nodes | jig edges | jig tree | grep -A 5 "Project"
```

## USAGE

### Full pipeline example
```bash
# Complete workflow from finding files to visualization
jig find . -p '\.md$' | jig filter | jig nodes | jig edges | jig tree
```

### Save tree to file
```bash
# Export tree structure to a text file
jig find ./notes | jig filter | jig nodes | jig edges | jig tree > hierarchy.txt
```

### Focus on specific paths
```bash
# Visualize only specific subdirectories
jig find ./docs -p '\.md$' | jig filter | jig nodes | jig edges | jig tree
```

## BEHAVIOR

**Root Nodes**: Nodes that have no parents (orphan nodes) are displayed at the root level of the tree.

**Multiple Roots**: If the knowledge graph has multiple disconnected hierarchies, each root will be displayed separately.

**Hierarchy Building**: The tree is constructed by processing edges and building parent-child relationships. Each node can have multiple children but only one parent (based on the parent link).

**Display Order**: Children are displayed in the order they were processed from the input.

**Unicode Support**: The tree uses Unicode box-drawing characters for proper visual rendering. Ensure your terminal supports UTF-8 encoding.

## INVOCATION MODES

**jig-tree** can be invoked in two ways:

1. **Subcommand mode**: `jig tree <args>` - called as a subcommand of the main `jig` executable
2. **Standalone mode**: `jig-tree <args>` - called as a standalone executable

Both modes provide identical functionality.
