#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "abi_dispatch_map.h"
#include "libantigravity_interop.h"

void run_extra_verification_tests2(void) {
    // 239. Verify Yul instruction opcode translation
    int translated_op = 0;
    assert(interop_yul_translate_opcode(100, &translated_op) == 0);
    assert(translated_op == 1);
    assert(interop_yul_translate_opcode(101, &translated_op) == 0);
    assert(translated_op == 2);
    assert(interop_yul_translate_opcode(105, &translated_op) == 0);
    assert(translated_op == 6);
    assert(interop_yul_translate_opcode(999, &translated_op) == -2);
    printf("✓ Yul opcode instruction translating verified.\n");

    // 240. Verify Yul object execution and memory mapping
    InteropStackVM vm;
    memset(&vm, 0, sizeof(InteropStackVM));
    int yul_bc[11] = { 100, 100, 100, 500, 106, 100, 200, 100, 700, 106, 105 };
    int memory_pages[10] = {0};
    size_t mem_count = 0;
    assert(interop_yul_execute_object(&vm, yul_bc, 11, memory_pages, &mem_count, 5) == 0);
    assert(mem_count == 2);
    assert(memory_pages[0] == 100 && memory_pages[1] == 500);
    assert(memory_pages[2] == 200 && memory_pages[3] == 700);
    printf("✓ Yul object execution on 2-stack VM verified.\n");

    // 241. Verify Yul memory state verification
    int pl_verified = -1;
    assert(interop_yul_verify_memory(memory_pages, mem_count, 100, 500, &pl_verified) == 0);
    assert(pl_verified == 1);
    assert(interop_yul_verify_memory(memory_pages, mem_count, 200, 700, &pl_verified) == 0);
    assert(pl_verified == 1);
    assert(interop_yul_verify_memory(memory_pages, mem_count, 100, 999, &pl_verified) == 0);
    assert(pl_verified == 0);
    printf("✓ Yul EVM compatibility memory proof verified.\n");
}
