# jig-slugify(1)

## NAME

**jig-slugify** - convert text to URL-friendly slugs

## SYNOPSIS

```
jig-slugify [OPTIONS] TEXT
```

## DESCRIPTION

**jig-slugify** transforms arbitrary text into URL-safe, human-readable slugs suitable for:
- File names
- URL paths
- Database keys
- Markdown anchors
- Directory names

Applies the following transformations:
1. Convert to lowercase
2. Remove non-ASCII characters (bytes > 127)
3. Replace spaces with hyphens
4. Remove all other non-alphanumeric characters
5. Collapse multiple hyphens into single hyphen
6. Trim leading/trailing hyphens

Output contains only: lowercase letters (a-z), digits (0-9), and hyphens (-)

This is a helper module designed for use in other jig components. The `slugify()` C function can be imported via `slugify/slugify.h` in other modules.

## OPTIONS

- **-h, --help** - Display help information and exit

## OUTPUT FORMAT

Slug format (variable length):
```
[a-z0-9]+(-[a-z0-9]+)*
```

Each slug contains only:
- Lowercase letters: a-z
- Digits: 0-9
- Hyphens: - (never at start/end, never consecutive)

Output is written to stdout with a trailing newline.

## EXIT STATUS

- **0** - Success (slug generated and output)
- **1** - Error (invalid arguments or memory allocation failure)

## EXAMPLES

Basic usage:
```bash
$ jig-slugify "Hello World"
hello-world

$ jig-slugify "My Blog Post"
my-blog-post
```

Uppercase conversion:
```bash
$ jig-slugify "UPPERCASE TEXT"
uppercase-text

$ jig-slugify "MiXeD CaSe"
mixed-case
```

Special character handling:
```bash
$ jig-slugify "Test@#$%123"
test123

$ jig-slugify "Hello, World!"
hello-world

$ jig-slugify "Test!!!???###"
test
```

Non-ASCII character removal:
```bash
$ jig-slugify "Café René"
caf-ren

$ jig-slugify "Zürich → München"
zrich-mnchen
```

Multiple spaces and hyphen collapsing:
```bash
$ jig-slugify "Too    Many     Spaces"
too-many-spaces

$ jig-slugify "---test---slug---"
test-slug
```

Edge cases:
```bash
$ jig-slugify "   test   "
test

$ jig-slugify "@@@test"
test

$ jig-slugify "test!!!"
test

$ jig-slugify ""
(empty output)

$ jig-slugify "@#$%"
(empty output)
```

Generate file names:
```bash
#!/bin/bash
title="My New Note"
slug=$(jig-slugify "$title")
filename="${slug}.md"
echo "Creating: $filename"  # Output: Creating: my-new-note.md
```

Batch file renaming:
```bash
for file in *.txt; do
    base=$(basename "$file" .txt)
    slug=$(jig-slugify "$base")
    mv "$file" "${slug}.txt"
done
```

Create directory from title:
```bash
project_name="My Awesome Project"
dir_name=$(jig-slugify "$project_name")
mkdir "$dir_name"  # Creates: my-awesome-project/
```

Generate URL paths:
```bash
page_title="Getting Started Guide"
url_path="/docs/$(jig-slugify "$page_title")"
echo "$url_path"  # Output: /docs/getting-started-guide
```

## BEHAVIOR

**Character Processing**:
- **Kept**: Letters (a-z after lowercasing), digits (0-9)
- **Replaced with hyphen**: Spaces (with collapsing)
- **Removed**: All other characters (punctuation, symbols, control chars, non-ASCII)

**Non-ASCII Handling**: Any byte with value > 127 is removed. This includes:
- Unicode characters (UTF-8 multibyte sequences)
- Extended ASCII characters
- Emoji and special symbols

**Edge Cases**:
- Empty input → empty output
- Only special characters → empty output
- Only spaces → empty output
- Leading/trailing hyphens → removed
- Multiple consecutive hyphens → collapsed to single hyphen
- Multiple consecutive spaces → collapsed to single hyphen

**Memory Safety**:
- Returns dynamically allocated string (caller must free)
- Returns NULL on allocation error (with error message to stderr)
- Returns empty string ("") for empty input

## LIMITATIONS

**Non-ASCII Support**: All non-ASCII characters are removed. This means:
- Unicode characters without ASCII equivalents are lost
- Diacritics are not transliterated (e.g., "café" → "caf", not "cafe")
- International scripts (Cyrillic, Arabic, Chinese, etc.) are completely removed
- Emoji and special symbols are removed

**Character Set**: Output is strictly ASCII alphanumeric plus hyphens. No other characters are preserved.

**Input Length**: No hard limit, but very long inputs may cause memory allocation to fail.

**Use Case**: Best suited for ASCII-based text. For international text requiring character transliteration, consider preprocessing with a Unicode normalization tool.

## INVOCATION MODES

**jig-slugify** is a standalone executable only:

```bash
jig-slugify [OPTIONS] TEXT
```

**Note**: Like uuid, slugify is **not** integrated as a subcommand of the main `jig` executable. It is a helper module for use in other components via the C API.

## C API USAGE

The slugify module can be used in other C programs:

```c
#include "slugify/slugify.h"

int main(void) {
    char *slug = slugify("Hello, World! 2024");
    if (slug != NULL) {
        printf("Slug: %s\n", slug);  // Output: hello-world-2024
        free(slug);  // Caller must free
    }
    return 0;
}
```

The `slugify()` function:
- Takes a NULL-terminated string
- Returns dynamically allocated slug string
- Returns NULL on allocation error (with error to stderr)
- Returns empty string for empty input
- Caller must free the returned string

Example with error handling:
```c
#include "slugify/slugify.h"
#include <stdio.h>
#include <stdlib.h>

char *slug = slugify(input_text);
if (slug == NULL) {
    fprintf(stderr, "Failed to create slug\n");
    return 1;
}

// Use the slug
printf("Generated slug: %s\n", slug);

// Always free when done
free(slug);
```

## BUILD

Build the standalone executable:

```bash
cd src/slugify
make
```

Binary created at `bin/jig-slugify`.

Clean build artifacts:

```bash
cd src/slugify
make clean
```

Rebuild:

```bash
cd src/slugify
make rebuild
```

## SEE ALSO

- RFC 3986: Uniform Resource Identifier (URI) syntax
- `/home/jedrasiak/jig/src/uuid/` - Similar helper module structure

## IMPLEMENTATION NOTES

**Algorithm**:
- Single-pass character processing
- In-place transformations to minimize allocations
- O(n) time complexity where n is input length
- Minimal memory footprint (one allocation, optimized at end)

**Compiler Flags**:
- Built with strict flags: `-Wall -Wextra -Werror`
- Ensures code quality and catches common mistakes

**Dependencies**:
- Standard C libraries only: `stdio.h`, `stdlib.h`, `string.h`
- No external dependencies required
