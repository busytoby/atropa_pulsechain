#!/bin/bash
set -e

# Robustly find the script's directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Move to firmware directory (2 levels up from notes/gemini)
cd "$DIR/../../"

echo "[BUILD] Compiling test_mathint with TSFi extensions from $(pwd)..."

# Compile with TSFi flags
gcc -o notes/gemini/test_tsfi_mathint \
    test_mathint.c \
    tsfi_lib/*.c \
    -I. \
    -Itsfi_lib \
    -DUSE_TSFI_MALLOC \
    -DUSE_TSFI_MATH \
    -std=gnu11 \
    -mavx512f -mavx512ifma -mavx512dq -mbmi2 \
    -pthread \
    -lm

echo "[BUILD] Success. Executable is at notes/gemini/test_tsfi_mathint"
