# jedrasiak:jig

## Unix philosophy
- Write programs that do one thing and do it well.
- Write programs to work together.
- Write programs to handle text streams, because that is a universal interface.

### Filtering files
```bash
find . -type f -iname "*.md"
find . -type f \( -iname "*.md" -o -iname "*.txt" \)
```

### Performance measuring
```bash
hyperfine "rg -l -i newag" "find . -type f \( -iname '*.md' -o -iname '*.txt' \) | jig-search-re 'newag'"
```

### Memory error detection
```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes bin/<COMMAND>
```

## Development

### Building from Source

The project uses Make for building. All source files are in `src/c/` and headers are in `include/`.

**Build the project:**
```bash
make
```

**Clean build artifacts:**
```bash
make clean
```

**Rebuild from scratch:**
```bash
make rebuild
```

**View build information:**
```bash
make info
```

### Manual Build (without Make)

If you prefer to compile manually:
```bash
gcc -Wall -Wextra -I./include src/c/main.c src/c/nv.c -o bin/jig
```