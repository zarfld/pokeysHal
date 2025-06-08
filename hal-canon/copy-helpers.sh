#!/bin/bash
# copy-helpers.sh - Copy hal_canon helper files into a HAL component source tree

set -e

# --- Configuration ---
FILENAME_PATTERN="hal_canon.h hal_*.c"

# --- Usage ---
if [ -z "$1" ]; then
    echo "Usage: $0 /path/to/component/source/"
    exit 1
fi

DEST="$1"

# --- Determine source directory ---
# Priority: local directory → pkg-config location
if ls $FILENAME_PATTERN >/dev/null 2>&1; then
    SRC="."
else
    echo "📦 Searching for installed helper source files via pkg-config..."
    SRC=$(pkg-config --variable=HelperSourceDir hal_canon 2>/dev/null || true)
    if [ -z "$SRC" ] || ! [ -d "$SRC" ]; then
        echo "❌ Could not find helper sources. Neither local nor system-installed found."
        exit 1
    fi
fi

echo "📁 Copying from: $SRC"
echo "📂 Copying to:   $DEST"

mkdir -p "$DEST"

for file in $FILENAME_PATTERN; do
    if [ -f "$SRC/$file" ]; then
        echo "→ $file"
        cp "$SRC/$file" "$DEST/"
    fi
done

echo "✅ Done. You can now #include \"hal_canon.h\" and the helper .c files in your HAL component."
