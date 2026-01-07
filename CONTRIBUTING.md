# Contributing to jig

Thank you for your interest in contributing to jig! This document provides guidelines and workflows for contributing to the project.

## Table of Contents

- [Getting Started](#getting-started)
- [Development Workflow](#development-workflow)
- [Branch Strategy](#branch-strategy)
- [Code Standards](#code-standards)
- [Commit Messages](#commit-messages)
- [Pull Request Process](#pull-request-process)
- [Testing](#testing)
- [Documentation](#documentation)

## Getting Started

### Prerequisites

- GCC compiler
- Make build tool
- Git
- Basic understanding of C and Unix philosophy

### Setting Up Development Environment

```bash
# Fork and clone the repository
git clone https://github.com/YOUR_USERNAME/jig.git
cd jig

# Add upstream remote
git remote add upstream https://github.com/ORIGINAL_OWNER/jig.git

# Create a development branch from dev
git checkout dev
git checkout -b feature/my-feature
```

### Building the Project

```bash
# Build the project
make

# Clean build artifacts
make clean

# Rebuild from scratch
make rebuild
```

See [CLAUDE.md](CLAUDE.md) for detailed build instructions and architecture overview.

## Development Workflow

1. **Fork the repository** on GitHub
2. **Clone your fork** locally
3. **Create a feature branch** from `dev`
4. **Make your changes** following code standards
5. **Test your changes** thoroughly
6. **Commit your changes** with clear messages
7. **Push to your fork**
8. **Open a Pull Request** to the `dev` branch

## Branch Strategy

### Main Branches

- **`main`**: Production-ready code, contains only releases
- **`dev`**: Integration branch for features and fixes

### Feature Branches

All contributions should follow this workflow:

```bash
# Always branch from dev
git checkout dev
git pull upstream dev
git checkout -b feature/descriptive-name

# Or for bug fixes
git checkout -b fix/issue-description
```

### Branch Naming Conventions

- **Features**: `feature/short-description`
- **Bug fixes**: `fix/issue-description`
- **Documentation**: `docs/what-changed`
- **Refactoring**: `refactor/what-changed`
- **Performance**: `perf/optimization-description`

## Code Standards

### C Style Guide

Follow these coding standards:

**Formatting:**
- Use 4 spaces for indentation (no tabs)
- Opening braces on the same line for functions
- Maximum line length: 100 characters
- Use descriptive variable names

**Example:**
```c
// Good
int parse_frontmatter(const char *content, Node *node) {
    if (!content || !node) {
        return -1;
    }

    // Implementation
    return 0;
}

// Avoid single-letter variables except for common idioms (i, j for loops)
for (int i = 0; i < count; i++) {
    // ...
}
```

**Memory Management:**
- Always free allocated memory
- Use `init_*()` and `cleanup_*()` patterns
- Check all allocations for NULL
- Avoid memory leaks (test with valgrind)

**Error Handling:**
- Return 0 for success, -1 or 1 for errors
- Check return values from system calls
- Provide meaningful error messages where appropriate

### Compiler Warnings

Code must compile without warnings:

```bash
gcc -Wall -Wextra -Werror -I./src ...
```

All warnings are treated as errors. Fix them before submitting.

### Unix Philosophy

Follow these principles:
- Do one thing well
- Work with text streams (stdin/stdout)
- Compose with other tools via pipes
- Silent operation (no unnecessary output)
- Exit status: 0 for success, non-zero for failure

## Commit Messages

Use [Conventional Commits](https://www.conventionalcommits.org/) format:

```
type(scope): short description

Longer explanation if needed.

Fixes #123
```

### Types

- **feat**: New feature
- **fix**: Bug fix
- **docs**: Documentation changes
- **refactor**: Code refactoring (no behavior change)
- **perf**: Performance improvements
- **test**: Adding or updating tests
- **build**: Build system changes
- **chore**: Maintenance tasks

### Examples

```
feat(nodes): add support for multi-line titles

Previously, titles were truncated at the first newline. This change
allows titles to span multiple lines in the YAML frontmatter.

Fixes #42
```

```
fix(filter): handle files with missing frontmatter

Files without YAML frontmatter now fail silently instead of crashing.

Closes #56
```

```
docs: update installation instructions in README
```

## Pull Request Process

### Before Submitting

- [ ] Code compiles without warnings
- [ ] All tests pass (if applicable)
- [ ] Memory leaks checked with valgrind
- [ ] Code follows style guidelines
- [ ] Commits follow conventional format
- [ ] Documentation updated (if needed)
- [ ] Changes tested manually

### Opening a Pull Request

1. **Target the `dev` branch** (not `main`)
2. **Fill out the PR template** with:
   - Clear description of changes
   - Motivation and context
   - Related issue numbers
   - Testing performed
3. **Keep PRs focused** - one feature/fix per PR
4. **Respond to feedback** - be open to suggestions

### PR Title Format

Use conventional commit format:

```
feat(nodes): add multi-line title support
fix(filter): handle missing frontmatter gracefully
docs: update contributing guidelines
```

### Review Process

- PRs are reviewed on the `dev` branch
- Maintainer will test and review your changes
- Address any requested changes
- Once approved, PR will be merged to `dev`
- Changes will reach `main` in the next release

## Testing

### Manual Testing

Test your changes with realistic data:

```bash
# Test with sample datasets
jig find datasets/ -p '\.md$' | jig filter | jig nodes | jig edges | jig tree

# Test edge cases
echo "test.md" | jig filter
jig find /nonexistent/path
```

### Memory Leak Testing

Use valgrind to check for memory leaks:

```bash
valgrind --leak-check=full --show-leak-kinds=all bin/jig find datasets/
```

All contributions should be leak-free.

### Module Testing

Test individual modules in isolation:

```bash
cd src/filter && make
./bin/filter-cli test.md

cd src/find && make
./bin/find-cli datasets/
```

## Documentation

### Code Documentation

- Add comments for complex logic
- Document public APIs in header files
- Keep README.md files updated in each module directory

### User Documentation

When adding new features:
- Update module README.md with usage examples
- Add examples to main README.md if applicable
- Update CLAUDE.md with architectural changes

## Questions?

- **Issues**: Open an issue for bugs or feature requests
- **Discussions**: Use GitHub Discussions for questions
- **Contact**: Reach out to maintainers via GitHub

## License

By contributing, you agree that your contributions will be licensed under the same license as the project.

---

Thank you for contributing to jig! Your efforts help make this tool better for everyone.
