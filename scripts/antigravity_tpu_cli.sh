#!/usr/bin/env bash
# Antigravity TPU Command-Line Interface Helper

# Ensure compile target exists
if [ ! -f "tools/antigravity_tpu_diagnostics" ]; then
    gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc tools/antigravity_tpu_diagnostics.c src/auncient_sdk.o -o tools/antigravity_tpu_diagnostics -lm
fi

if [ -z "$1" ]; then
    echo "Usage: $0 \"WORD [name] [id] [f1] [f2] [decay] [wmq_hex] [abi_hex]\""
    exit 1
fi

# Build input source script from command line arguments
SCRIPT_CONTENT="# Stdin diagnostic script
$1"

# Pipe to the diagnostic binary
echo "$SCRIPT_CONTENT" | ./tools/antigravity_tpu_diagnostics
