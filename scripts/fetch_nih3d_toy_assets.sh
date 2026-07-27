#!/bin/bash
# Generate clean local Auncient mock toy bear STL mesh components for deterministic builds

set -e

ASSETS_DIR="tsfi2-deepseek/assets"
mkdir -p "$ASSETS_DIR"

echo "[GENERATE] Generating deterministic Auncient toy bear STL components..."

python3 -c '
import struct

# Write toy_bear_head.stl
with open("'"$ASSETS_DIR"'/toy_bear_head.stl", "wb") as f:
    f.write(b"Auncient Mock Head STL Header".ljust(80, b"\0"))
    f.write(struct.pack("<I", 1))
    f.write(struct.pack("<12fH", 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0))

# Write toy_bear_joint.stl
with open("'"$ASSETS_DIR"'/toy_bear_joint.stl", "wb") as f:
    f.write(b"Auncient Mock Joint STL Header".ljust(80, b"\0"))
    f.write(struct.pack("<I", 1))
    f.write(struct.pack("<12fH", 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0))
'

echo "[SUCCESS] Deterministic toy bear STL meshes generated in $ASSETS_DIR"
