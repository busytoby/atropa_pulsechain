#!/bin/bash
XGCC="./tools/gcc-fork/build-gcc-tsfi/gcc/xgcc"
SPECS="-B./tools/gcc-fork/build-gcc-tsfi/gcc/"
LIBGCC="./tools/gcc-fork/build-gcc-tsfi/x86_64-pc-linux-gnu/libgcc/libgcc.a"
CFLAGS="-Iinc -march=native -O3 -D_POSIX_C_SOURCE=200809L -fPIE -mavx512f -mavx512dq -mavx512vl -mavx512bw -mavx512cd -mavx512vnni -mavx512ifma"

echo "Compiling Dependencies..."
$XGCC $SPECS $CFLAGS -c src/lau_memory.c -o obj/lau_memory.o
$XGCC $SPECS $CFLAGS -c src/lau_registry.c -o obj/lau_registry.o
$XGCC $SPECS $CFLAGS -c src/lau_audit.c -o obj/lau_audit.o
$XGCC $SPECS $CFLAGS -c src/lau_thunk.c -o obj/lau_thunk.o
$XGCC $SPECS $CFLAGS -c src/tsfi_font_ai.c -o obj/tsfi_font_ai.o
$XGCC $SPECS $CFLAGS -c src/tsfi_wiring.c -o obj/tsfi_wiring.o
$XGCC $SPECS $CFLAGS -c src/tsfi_logic.c -o obj/tsfi_logic.o
$XGCC $SPECS $CFLAGS -c src/tsfi_trace.c -o obj/tsfi_trace.o
$XGCC $SPECS $CFLAGS -c src/tsfi_hilbert_avx512.c -o obj/tsfi_hilbert.o
$XGCC $SPECS $CFLAGS -c src/tsfi_hotloader.c -o obj/tsfi_hotloader.o
$XGCC $SPECS $CFLAGS -c src/tsfi_math.c -o obj/tsfi_math.o
$XGCC $SPECS $CFLAGS -c src/tsfi_opt_zmm.c -o obj/tsfi_opt_zmm.o
$XGCC $SPECS $CFLAGS -c src/tsfi_svdag.c -o obj/tsfi_svdag.o
$XGCC $SPECS $CFLAGS -c src/tsfi_tessellator.c -o obj/tsfi_tessellator.o
$XGCC $SPECS $CFLAGS -c src/tsfi_vision.c -o obj/tsfi_vision.o

echo "Compiling Benchmark..."
$XGCC $SPECS $CFLAGS -c tests/bench_svdag_transcendent.c -o obj/bench_svdag.o

echo "Linking..."
gcc obj/bench_svdag.o obj/lau_memory.o obj/lau_registry.o obj/lau_audit.o obj/lau_thunk.o obj/tsfi_font_ai.o obj/tsfi_wiring.o obj/tsfi_logic.o obj/tsfi_trace.o obj/tsfi_hilbert.o obj/tsfi_hotloader.o obj/tsfi_math.o obj/tsfi_opt_zmm.o obj/tsfi_svdag.o obj/tsfi_tessellator.o obj/tsfi_vision.o $LIBGCC -lpthread -ldl -lm -o tests/bench_transcendent

echo "Running Benchmark..."
./tests/bench_transcendent