# jig-hierarchy(1)

## NAME

**jig-hierarchy** - generate ancestor chain data for breadcrumb navigation

## SYNOPSIS

```
jig hierarchy [OPTIONS]
jig-hierarchy [OPTIONS]
```

## DESCRIPTION

**jig-hierarchy** reads file paths from stdin, extracts node information from each file, builds parent-child relationships from the parent links, and outputs a YAML file mapping each note's slug to its ancestor chain.

This command is designed to solve the breadcrumbs problem in flat folder structures where logical hierarchy is defined via links front matter with `?label=parent` query parameter. The output is suitable for Hugo's `data/` directory.

## OPTIONS

- **-h, --help** - Display help information and exit

## INPUT FORMAT

Expects file paths from stdin (one per line).
Typically used after jig-filter to read validated note files.

Each file should have YAML frontmatter with:
- `id` - Unique identifier
- `title` - Node title
- Optional parent link: `[text](path?label=parent)`

## OUTPUT FORMAT

YAML format mapping slugs to ancestor arrays:

```yaml
binary:
  ancestors:
    - place-value-notation
    - numeral-systems
    - mathematics
    - foundations
    - engineering
place-value-notation:
  ancestors:
    - numeral-systems
    - mathematics
    - foundations
    - engineering
engineering:
  ancestors: []
```

**Key characteristics:**
- Each entry key is the note's slug (derived from folder name)
- Ancestors are ordered from immediate parent to root (most specific first)
- Root nodes have empty ancestors: `[]`

## SLUG DERIVATION

For English files (pattern `*.en.md`):
- Uses the folder name containing the file
- Example: `/content/cse/binary/index.en.md` → `binary`

For non-English files:
- Currently uses folder name (same as English)
- Future: could use `slug` from front matter or `slugify(title)` as fallback

## EXIT STATUS

- **0** - Success (hierarchy data generated)
- **1** - Error (memory allocation failure or invalid input)

## EXAMPLES

Generate English hierarchy data:
```bash
$ jig find . -p '\.en.md$' | jig filter | jig hierarchy
binary:
  ancestors:
    - place-value-notation
    - numeral-systems
    - mathematics
engineering:
  ancestors: []
```

Save hierarchy data to Hugo's data directory:
```bash
$ cd content/cse
$ jig find -p ".*\.en.md$" | jig filter | jig hierarchy > ../data/cse_hierarchy_en.yaml
$ jig find -p ".*\.pl.md$" | jig filter | jig hierarchy > ../data/cse_hierarchy_pl.yaml
```

## USAGE

### Integration with Hugo

The output is designed for Hugo's data directory. In Hugo templates:

```go-html-template
{{ $hierarchy := index $.Site.Data "cse_hierarchy_en" }}
{{ $slug := .File.ContentBaseName }}
{{ $entry := index $hierarchy $slug }}
{{ if $entry }}
  {{ range $entry.ancestors }}
    {{/* render breadcrumb item */}}
  {{ end }}
{{ end }}
```

### Build script integration

Add to your build script (e.g., `scripts/cse.sh`):

```bash
#!/bin/bash
cd content/cse

# Generate tree (for reference)
jig find -p ".*\.en.md$" | jig filter | jig tree

# Generate hierarchy data for breadcrumbs
mkdir -p ../data
jig find -p ".*\.en.md$" | jig filter | jig hierarchy > ../data/cse_hierarchy_en.yaml
jig find -p ".*\.pl.md$" | jig filter | jig hierarchy > ../data/cse_hierarchy_pl.yaml
```

### Full pipeline example

```bash
# Complete workflow from finding files to hierarchy data
jig find . -p '\.en.md$' | jig filter | jig hierarchy > hierarchy.yaml
```

## BEHAVIOR

**Slug Extraction**: Slugs are derived from the parent folder name in the file path. For `/content/cse/binary/index.en.md`, the slug is `binary`.

**Parent Resolution**: Parent relationships are determined by parsing links with `?label=parent` query parameter in the markdown content, following the same logic as `jig tree`.

**Ancestor Order**: Ancestors are listed from nearest (immediate parent) to farthest (root). This allows templates to render breadcrumbs in the correct order by reversing the array.

**Root Nodes**: Nodes without parents have an empty ancestors array (`ancestors: []`).

**Deduplication**: Each note appears exactly once in the output, keyed by its slug.

## RELATIONSHIP TO OTHER COMMANDS

**jig-hierarchy** shares parsing logic with **jig-tree**:
- Both read file paths from stdin
- Both use the same parent link detection (`?label=parent`)
- Both build the same internal node and edge structures

The difference is in output:
- **jig-tree** outputs ASCII tree visualization
- **jig-hierarchy** outputs YAML data for programmatic use

## INVOCATION MODES

**jig-hierarchy** can be invoked in two ways:

1. **Subcommand mode**: `jig hierarchy <args>` - called as a subcommand of the main `jig` executable
2. **Standalone mode**: `jig-hierarchy <args>` - called as a standalone executable

Both modes provide identical functionality.
