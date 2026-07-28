#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_strategy_lang.h"
#include "tsfi_anvil_vm.h"

// Expose internal compilation for debugging
int tsfi_strategy_compile_script(const char *script, uint8_t *bytecode_out, int max_len, int *len_out);

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COBOL STRATEGY TEST RUNNER\n");
    printf("=============================================================\n");

    // Read the strategy file content
    FILE *f = fopen("solidity/dysnomia/domain/strategies/tape_loader.strategy", "r");
    if (!f) {
        f = fopen("../solidity/dysnomia/domain/strategies/tape_loader.strategy", "r");
    }
    if (!f) {
        fprintf(stderr, "FATAL: Could not open tape_loader.strategy from root or subfolder\n");
        return 1;
    }

    char script[1024];
    size_t bytes_read = fread(script, 1, sizeof(script) - 1, f);
    script[bytes_read] = '\0';
    fclose(f);

    printf("[DEBUG] Script content:\n%s\n", script);

    // Compile and print bytecode
    uint8_t bc[256];
    int bc_len = 0;
    int compile_res = tsfi_strategy_compile_script(script, bc, 256, &bc_len);
    printf("[DEBUG] Compile result: %d, Bytecode len: %d\n", compile_res, bc_len);
    printf("[DEBUG] Bytecode: ");
    for (int i = 0; i < bc_len; i++) {
        printf("%02X ", bc[i]);
    }
    printf("\n");

    // Initialize Strategy VM and Priority Queue
    TSFiStrategyVM vm;
    tsfi_strategy_vm_init(&vm);

    TSFiPriorityQueue pq;
    tsfi_priority_queue_init(&pq);

    printf("[TEST] Executing COBOL word alteration strategy script...\n");
    int res = tsfi_strategy_vm_execute(&vm, &pq, script, NULL);
    assert(res == 0);

    // Verify VM state alterations
    printf("   ✓ Depth Scale: %d (Expected: 5)\n", vm.depth_priority_scale);
    printf("   ✓ Abductive Scale: %d (Expected: 8)\n", vm.abductive_priority_scale);
    assert(vm.depth_priority_scale == 5);
    assert(vm.abductive_priority_scale == 8);

    printf("=============================================================\n");
    printf("COBOL STRATEGY TESTS COMPLETED\n");
    printf("=============================================================\n");

    return 0;
}
