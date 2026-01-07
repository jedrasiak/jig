# jig-nodes(1)

## NAME

**jig-nodes** - extract and list nodes from note files

## SYNOPSIS

```
jig nodes [OPTIONS]
jig-nodes [OPTIONS]
```

## DESCRIPTION

**jig-nodes** reads file paths from stdin, parses each file to extract node information (id, title, link), and outputs the data as CSV format.

A valid node file must contain YAML frontmatter with:
- id: A unique identifier (max 36 bytes)
- title: The node title
- (optional) A parent link in the format: [text](path?label=parent)

## OPTIONS

- **-h, --help** - Display help information and exit

## OUTPUT FORMAT

CSV format with the following columns:
- **id** - Unique identifier from frontmatter
- **title** - Node title from frontmatter
- **path** - File path
- **link** - Parent link path (if present)

## EXIT STATUS

- **0** - Success
- **1** - Error (invalid regex, memory allocation failure, or file processing error)

## EXAMPLES

Extract nodes from markdown files:
```bash
$ jig find . -p '\.md$' | jig filter | jig nodes
id,title,path,link
abc123,Project Overview,./notes/project.md,
def456,Task List,./notes/tasks.md,./notes/project.md
```

Use with other commands:
```bash
# Count total nodes
jig find . | jig filter | jig nodes | tail -n +2 | wc -l

# Extract just titles
jig find . | jig filter | jig nodes | tail -n +2 | cut -d, -f2
```

## INVOCATION MODES

**jig-nodes** can be invoked in two ways:

1. **Subcommand mode**: `jig nodes <args>` - called as a subcommand of the main `jig` executable
2. **Standalone mode**: `jig-nodes <args>` - called as a standalone executable

Both modes provide identical functionality.
