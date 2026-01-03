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

## SEE ALSO

jig-find(1), jig-filter(1), jig-edges(1), jig-tree(1)
