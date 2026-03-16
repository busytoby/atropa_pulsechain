#!/bin/bash
# TSFi JIT Thunk Compiler
# Usage: ./tsfi_compile_thunk.sh <xi_val> <ohm_val> <rho_val> <output_bin>

if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <xi_val> <ohm_val> <rho_val> <output_bin>"
    exit 1
fi

XI_VAL=$1
OHM_VAL=$2
RHO_VAL=$3
OUT_BIN=$4

TMP_SRC="/tmp/tsfi_dynamic_thunk_$$.c"
TMP_OBJ="/tmp/tsfi_dynamic_thunk_$$.o"

# Clone the template and inject the mathematical vectors
sed -e "s/0x0, 0, 0, 0, 0, 0, 0, 0/$XI_VAL, 0, 0, 0, 0, 0, 0, 0/g" 
    -e "s/0x0, 0, 0, 0, 0, 0, 0, 0/$OHM_VAL, 0, 0, 0, 0, 0, 0, 0/g" 
    -e "s/0x0, 0, 0, 0, 0, 0, 0, 0/$RHO_VAL, 0, 0, 0, 0, 0, 0, 0/g" 
    thunks/dysnomia/fuse_payload_template.c > $TMP_SRC

# Compile to position-independent raw object code
# -fPIC ensures it can be executed anywhere in the Firmware's mapped memory
# -Os optimizes for size (minimal footprint)
gcc -O3 -fPIC -fno-stack-protector -fno-asynchronous-unwind-tables -c $TMP_SRC -o $TMP_OBJ

if [ $? -ne 0 ]; then
    echo "FATAL: Thunk compilation failed."
    rm -f $TMP_SRC
    exit 1
fi

# Extract strictly the executable .text section into a flat binary file
objcopy -O binary -j .text $TMP_OBJ $OUT_BIN

if [ $? -ne 0 ]; then
    echo "FATAL: Binary extraction failed."
    rm -f $TMP_SRC $TMP_OBJ
    exit 1
fi

# Clean up
rm -f $TMP_SRC $TMP_OBJ
echo "SUCCESS: $OUT_BIN compiled."
exit 0
