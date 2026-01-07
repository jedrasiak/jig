#!/bin/bash
# Adds the project's /bin directory to the PATH in ~/.bashrc if not already added

# --- CONFIG ---
# Automatically detect the repo root (script is in scripts/ directory)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BIN_DIR="$REPO_DIR/bin"
BASHRC="$HOME/.bashrc"

# --- CHECKS ---
if [ ! -d "$BIN_DIR" ]; then
  echo "Warning: bin directory does not exist yet. Creating it..."
  mkdir -p "$BIN_DIR"
fi

# The export line we want to ensure exists in .bashrc
EXPORT_LINE="export PATH=\"\$PATH:$BIN_DIR\""

# --- APPLY CHANGES ---
# Only add the line if it's not already in .bashrc
if ! grep -Fxq "$EXPORT_LINE" "$BASHRC"; then
  echo -e "\n# Added by jig setup script" >> "$BASHRC"
  echo "$EXPORT_LINE" >> "$BASHRC"
  echo "Added $BIN_DIR to PATH in $BASHRC"
else
  echo "$BIN_DIR already in PATH (nothing changed)"
fi

# Reload shell config
echo "Reloading shell..."
source "$BASHRC"

echo "Done! You can now run binaries from:"
echo "$BIN_DIR"
