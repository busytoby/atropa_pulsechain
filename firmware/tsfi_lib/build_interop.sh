#!/bin/bash
set -e

# Directory of this script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$DIR"

echo "[BUILD] Compiling libtsfi_interop.so..."

# Compile shared library
gcc -O3 -mavx512ifma -mavx512vl -Wall -Wextra -fPIC -shared \
    -o libtsfi_interop.so \
    tsfi_interop.c \
    lau_memory.c \
    lau_registry.c \
    tsfi_log.c \
    tsfi_math.c \
    tsfi_wiring.c \
    tsfi_logic.c \
    lau_thunk.c \
    -I. \
    -ldl

echo "[BUILD] Success."

# Deploy to Dysnomia output
# Note: Adjust paths as needed based on where .NET build outputs are
DEST1="../../../linux/bin/Debug/net10.0"
DEST2="../../../Dysnomia/bin/Debug/net10.0"

mkdir -p "$DEST1"
mkdir -p "$DEST2"

cp libtsfi_interop.so "$DEST1/"
cp libtsfi_interop.so "$DEST2/"

# Deploy to Dysnomia project root for inclusion
DEST3="../../../Dysnomia"
cp libtsfi_interop.so "$DEST3/"

echo "[DEPLOY] Copied to $DEST1, $DEST2, and $DEST3"