# jig-filter(1)

## NAME

**jig-filter** - filter and validate note files based on frontmatter criteria

## SYNOPSIS

```
jig filter <filepath>
jig-filter <filepath>
<command> | jig filter
<command> | jig-filter
```

## DESCRIPTION

**jig-filter** filters note files by validating their YAML frontmatter structure. It processes filepaths from either command-line arguments or standard input (piped commands) and outputs only those files that meet the specified criteria.

A valid note file must contain:
- YAML frontmatter delimited by `---` at the beginning of the file
- An `id` property in the frontmatter (maximum 36 bytes)
- A `title` property in the frontmatter

Files that do not meet these requirements are silently filtered out.

## USAGE

### Process a single file
```bash
jig filter path/to/note.md
jig-filter path/to/note.md
```

### Filter files from piped input
```bash
find . -name "*.md" | jig filter
ls -1 notes/*.md | jig-filter
```

### Combine with other tools
```bash
# Find and validate all markdown files
find . -type f -name "*.md" | jig filter

# Validate files modified in the last 7 days
find . -name "*.md" -mtime -7 | jig filter
```

## EXIT STATUS

- **0** - Success (valid files found or processed)
- **1** - Error (no input provided or processing failure)

## EXAMPLES

Process a specific note file:
```bash
$ jig filter notes/meeting-2024.md
notes/meeting-2024.md
```

Filter multiple files through a pipe:
```bash
$ find notes/ -name "*.md" | jig filter
notes/project-a.md
notes/summary.md
```

## INVOCATION MODES

**jig-filter** can be invoked in two ways:

1. **Subcommand mode**: `jig filter <args>` - called as a subcommand of the main `jig` executable
2. **Standalone mode**: `jig-filter <args>` - called as a standalone executable

Both modes provide identical functionality.
