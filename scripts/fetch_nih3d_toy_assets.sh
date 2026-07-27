#!/bin/bash
# Generate clean local Auncient mock toy bear STL mesh components for deterministic builds

set -e

ASSETS_DIR="tsfi2-deepseek/assets"
mkdir -p "$ASSETS_DIR"

echo "[GENERATE] Generating deterministic Auncient toy bear STL components..."

gcc -Wall -Wextra -Werror -std=c11 -O3 tools/generate_bear_stls.c -o tools/generate_bear_stls -lm
./tools/generate_bear_stls
rm -f tools/generate_bear_stls

echo "[SUCCESS] Volumetric toy bear STL meshes generated in $ASSETS_DIR"
