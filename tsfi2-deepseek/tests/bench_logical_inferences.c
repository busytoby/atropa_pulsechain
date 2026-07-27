#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tsfi_anvil_vm.h"

#define ITERATIONS 5000000

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ANVIL VM: LOGICAL INFERENCES BENCHMARK\n");
    printf("=============================================================\n");

    TSFiAnvilVM vm;
    tsfi_anvil_vm_init(&vm);

    // Setup coordinates for implication resolution
    tsfi_anvil_vm_bind(&vm, "source_a", "TRUE");
    tsfi_anvil_vm_bind(&vm, "source_b", "TRUE");
    tsfi_anvil_vm_bind(&vm, "target_c", "FALSE");

    int slot_a = -1, slot_b = -1, slot_dst = -1;
    for (int i = 0; i < 64; i++) {
        if (strcmp(vm.subgoal_table[i].key, "source_a") == 0) slot_a = i;
        if (strcmp(vm.subgoal_table[i].key, "source_b") == 0) slot_b = i;
        if (strcmp(vm.subgoal_table[i].key, "target_c") == 0) slot_dst = i;
    }

    if (slot_a == -1 || slot_b == -1 || slot_dst == -1) {
        printf("Error: Could not map subgoal registers.\n");
        return 1;
    }

    // Bytecode program: RESOLVE_IMPLICATION slot_a slot_b slot_dst
    int bytecode[4] = { 0x5C, slot_a, slot_b, slot_dst };

    printf("Running %d logical inference cycles (function call)...\n", ITERATIONS);
    fflush(stdout);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < ITERATIONS; i++) {
        vm.subgoal_table[slot_dst].value[0] = 'F';
        tsfi_anvil_vm_execute(&vm, bytecode, 4, 1.0f, 1.0f, 0.0f);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_func = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Running %d logical inference cycles (inline hot-path)...\n", ITERATIONS);
    fflush(stdout);

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < ITERATIONS; i++) {
        vm.subgoal_table[slot_dst].value[0] = 'F';
        if (vm.subgoal_table[slot_a].value[0] == 'T' && vm.subgoal_table[slot_b].value[0] == 'T') {
            vm.subgoal_table[slot_dst].value[0] = 'T';
            vm.subgoal_table[slot_dst].value[1] = 'R';
            vm.subgoal_table[slot_dst].value[2] = 'U';
            vm.subgoal_table[slot_dst].value[3] = 'E';
            vm.subgoal_table[slot_dst].value[4] = '\0';
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_inline = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    double lips_func = ITERATIONS / elapsed_func;
    double lips_inline = ITERATIONS / elapsed_inline;

    printf("\nBenchmark Results:\n");
    printf("  [Function Call Mode] Elapsed: %.4f s | LIPS: %.2f M LIPS\n", elapsed_func, lips_func / 1e6);
    printf("  [Inline Hot-Path Mode] Elapsed: %.4f s | LIPS: %.2f M LIPS (%.2f G LIPS)\n", elapsed_inline, lips_inline / 1e6, lips_inline / 1e9);
    printf("=============================================================\n");

    return 0;
}
