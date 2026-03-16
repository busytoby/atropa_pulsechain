#!/bin/bash

TARGET="atropa_pulsechain/tsfi2-deepseek"

echo "Preparing release in $TARGET..."

# Ensure target exists and is clean
rm -rf "$TARGET"/*
mkdir -p "$TARGET"

# Copy source and headers
cp -r src "$TARGET/"
cp -r inc "$TARGET/"
cp -r plugins "$TARGET/"
cp -r tests "$TARGET/"
cp -r experiments "$TARGET/"
cp -r thunks "$TARGET/"
cp -r firmware "$TARGET/"

# Copy build and documentation files
cp Makefile "$TARGET/"
cp *.md "$TARGET/" 2>/dev/null || true
cp *.sh "$TARGET/" 2>/dev/null || true
cp *.py "$TARGET/" 2>/dev/null || true

# Copy local non-downloadable assets (ignoring large model weights, etc.)
mkdir -p "$TARGET/assets"
if [ -d "assets" ]; then
    rsync -av --exclude='models/*.safetensors' --exclude='models/*.bin' --exclude='models/*.pth' assets/ "$TARGET/assets/"
fi

# Copy tools
if [ -d "tools" ]; then
    cp -r tools "$TARGET/"
fi

# Clean up unwanted artifacts from the target directory
find "$TARGET" -type d -name "obj" -exec rm -rf {} +
find "$TARGET" -type d -name "bin" -exec rm -rf {} +
find "$TARGET" -type d -name "__pycache__" -exec rm -rf {} +
find "$TARGET" -name "*.o" -type f -delete
find "$TARGET" -name "*.out" -type f -delete
find "$TARGET" -name "*.so" -type f -delete
find "$TARGET" -name "*.log" -type f -delete
find "$TARGET" -name "*.pos" -type f -delete
find "$TARGET" -name "core" -type f -delete

echo "Release preparation complete."
