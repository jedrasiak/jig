# jig-find(1)

## NAME

**jig-find** - recursively find and list files in a directory tree

## SYNOPSIS

```
jig find [OPTIONS] [PATH]
jig-find [OPTIONS] [PATH]
```

## DESCRIPTION

**jig-find** recursively traverses directories and outputs the full path of every file found. It provides a simple, focused alternative to the standard `find` command with sensible defaults for working with note collections and project files.

The tool automatically:
- Recursively walks directory trees up to 100 levels deep
- Skips hidden files and directories (starting with `.`)
- Skips symbolic links to prevent infinite loops
- Outputs one file path per line to stdout
- Supports optional regex pattern filtering on filenames

If no path is provided, it searches the current directory by default.

## OPTIONS

- **-h, --help** - Display help information and exit
- **-p, --pattern PATTERN** - Filter files by regex pattern (matches filename only, not full path)

## USAGE

### Find all files in current directory
```bash
jig find
jig-find
```

### Find files in a specific directory
```bash
jig find ./notes
jig-find path/to/directory
```

### Filter files by pattern
```bash
# Find only markdown files
jig find -p '\.md$' ./notes

# Find files starting with 'test'
jig find -p '^test' ./src

# Find files containing 'config'
jig find -p 'config'
```

### Pipe to other tools
```bash
# Find and validate note files
jig find ./notes | jig filter

# Find markdown files and count them
jig find -p '\.md$' | wc -l

# Find and search in files
jig find ./notes | xargs grep "keyword"
```

## EXIT STATUS

- **0** - Success (files found and listed, or no errors during traversal)
- **1** - Error (directory cannot be opened, invalid regex pattern, or other processing failure)

## EXAMPLES

List all files in the notes directory:
```bash
$ jig find notes/
notes/project-a.md
notes/meeting-2024.md
notes/summary.md
notes/archive/old-notes.md
```

Find only markdown files:
```bash
$ jig find -p '\.md$' notes/
notes/project-a.md
notes/meeting-2024.md
notes/summary.md
```

Find and filter valid note files:
```bash
$ jig find ./notes | jig filter
notes/project-a.md
notes/summary.md
```

Find files with specific naming pattern:
```bash
$ jig find -p '^index\.' ./content
content/section1/index.en.md
content/section2/index.en.md
```

Count total files in current directory:
```bash
$ jig find | wc -l
127
```

## BEHAVIOR

**Hidden Files**: Files and directories starting with `.` are automatically skipped. This includes `.git`, `.DS_Store`, and similar metadata files.

**Symbolic Links**: Symbolic links are detected using `lstat()` and skipped to prevent infinite loops and circular references in the directory tree.

**Depth Limit**: Recursion is limited to 100 directory levels to prevent stack overflow from pathological directory structures.

**Path Construction**: Handles paths with and without trailing slashes correctly to avoid double-slash artifacts in output.

**Pattern Matching**: When using `-p/--pattern`, the regex is matched against the filename only (not the full path). Patterns use extended regex syntax and are case-insensitive.

## PATTERN SYNTAX

Patterns use POSIX Extended Regular Expression (ERE) syntax:

- `.` - Match any character
- `^` - Match start of filename
- `$` - Match end of filename
- `*` - Match zero or more of previous
- `+` - Match one or more of previous
- `[abc]` - Match any character in set
- `[^abc]` - Match any character not in set
- `\.` - Escape special characters

Examples:
- `\.md$` - Files ending with .md
- `^test` - Files starting with 'test'
- `^[0-9]` - Files starting with a digit
- `\.(md|txt)$` - Files ending with .md or .txt

## INVOCATION MODES

**jig-find** can be invoked in two ways:

1. **Subcommand mode**: `jig find <args>` - called as a subcommand of the main `jig` executable
2. **Standalone mode**: `jig-find <args>` - called as a standalone executable

Both modes provide identical functionality.

## SEE ALSO

jig(1), jig-filter(1), jig-tree(1), find(1)
