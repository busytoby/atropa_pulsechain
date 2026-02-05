#!/bin/bash
set -e

# Move to firmware root
cd "$(dirname "$0")/../.."

echo "--- Building Baseline (Legacy Gemalloc/MathInt) ---"
gcc -O2 -Wall -Wextra -o test_mathint test_mathint.c
./test_mathint
echo "---------------------------------------------------"

echo "--- Building TSFi Integrated (LauMemory/TsfiMath) ---"
# Define flags to enable TSFi integration in headers
# Link against tsfi_lib sources
gcc -O3 -mavx512ifma -mavx512vl -Wall -Wextra \
    -DUSE_TSFI_MALLOC -DUSE_TSFI_MATH \
    -I. -Itsfi_lib \
    -o test_tsfi_mathint test_mathint.c \
    tsfi_lib/lau_memory.c \
    tsfi_lib/lau_registry.c \
    tsfi_lib/tsfi_log.c \
    tsfi_lib/tsfi_math.c \
    tsfi_lib/tsfi_wiring.c \
    tsfi_lib/tsfi_logic.c \
    tsfi_lib/lau_thunk.c \
    -ldl -lubsan

./test_tsfi_mathint
echo "---------------------------------------------------"

# Cleanup
rm test_mathint test_tsfi_mathint