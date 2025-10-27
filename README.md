# scientia

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

### Project Structure

```
scientia/
├── include/          # Header files (.h)
│   └── nn.h
├── src/c/            # Source files (.c)
│   ├── main.c
│   └── nn.c
├── build/            # Object files (.o) - generated
├── bin/              # Compiled executable - generated
│   └── scientia
└── Makefile          # Build configuration
```

### Manual Build (without Make)

If you prefer to compile manually:
```bash
gcc -Wall -Wextra -I./include src/c/main.c src/c/nn.c -o bin/scientia
```