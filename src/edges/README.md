# jig-edges(1)

## NAME

**jig-edges** - extract and list edges (relationships) between nodes

## SYNOPSIS

```
jig edges [OPTIONS]
jig-edges [OPTIONS]
```

## DESCRIPTION

**jig-edges** reads node CSV from stdin, builds relationships between nodes based on their links, and outputs edge information as CSV format.

An edge represents a relationship between two nodes. The command matches each node's parent link to find the corresponding parent node and creates an edge with label "parent".

## OPTIONS

- **-h, --help** - Display help information and exit

## INPUT FORMAT

Expects CSV from stdin with columns: id,title,path,link
(This is the output format from jig-nodes)

## OUTPUT FORMAT

CSV format with the following columns:
- **src_id** - Source node ID
- **src_title** - Source node title
- **dst_id** - Destination node ID
- **dst_title** - Destination node title
- **label** - Edge label (e.g., "parent")
- **src_path** - Source node file path
- **dst_path** - Destination node file path

## EXIT STATUS

- **0** - Success
- **1** - Error (memory allocation failure or invalid input)

## EXAMPLES

Extract edges from markdown files:
```bash
$ jig find . -p '\.md$' | jig filter | jig nodes | jig edges
src_id,src_title,dst_id,dst_title,label,src_path,dst_path
def456,Task List,abc123,Project Overview,parent,./notes/tasks.md,./notes/project.md
```

Use with other commands:
```bash
# Count total edges
jig find . | jig filter | jig nodes | jig edges | tail -n +2 | wc -l

# Extract only parent relationships
jig find . | jig filter | jig nodes | jig edges | grep ",parent,"
```

## INVOCATION MODES

**jig-edges** can be invoked in two ways:

1. **Subcommand mode**: `jig edges <args>` - called as a subcommand of the main `jig` executable
2. **Standalone mode**: `jig-edges <args>` - called as a standalone executable

Both modes provide identical functionality.
