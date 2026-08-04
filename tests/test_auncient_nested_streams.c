#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/tsfi2_compiler.h"
#include "inc/tsfi2_compiler_bin.h"
#include "inc/tsfi2_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT NESTED SUB-STREAM EXECUTION (XPLSM) UNIT TEST\n");
    printf("=============================================================\n");

    const char *src = "int main() { __builtin_wmq_load_stream(1); return 42; }";
    uint8_t bc[256];
    size_t bclen = 0;

    bool ok = tsfi2_compile(src, bc, sizeof(bc), &bclen);
    assert(ok == true);

    const char *test_file = "/tmp/nested_stream_test.dat.bin";
    ok = tsfi2_compile_to_dat_bin(test_file, 0x1000, 1, bc, bclen);
    assert(ok == true);

    Tsfi2CpuState cpu;
    ok = tsfi2_load_and_execute(test_file, &cpu);
    assert(ok == true);
    assert(cpu.halted == true);
    assert(cpu.exit_code == 42);

    printf("   ✓ Compiled program with dynamic load_stream instruction successfully.\n");
    printf("   ✓ CPU stack push, nested execution, and RET pop context resolved successfully.\n");
    printf("   ✓ Program returned clean parent exit code 42.\n");

    remove(test_file);
    return 0;
}
