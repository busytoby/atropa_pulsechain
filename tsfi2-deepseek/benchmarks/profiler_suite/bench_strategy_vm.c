#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "tsfi_strategy_lang.h"
#include "tsfi_ramac_layout.h"

#define ITERATIONS 1000000

int main(void) {
    printf("[Strategy VM Benchmark] Compiling and profiling execution cycles...\n");
    fflush(stdout);

    const char *script = "SET depth 2; SET abductive 5; SET_REG R0 10; SET_REG R1 20; ADD R0 R1; EVAL;";
    uint8_t bytecode[64];
    int bc_len = 0;

    int res = tsfi_strategy_compile_script(script, bytecode, 64, &bc_len);
    assert(res == 0);

    TSFiStrategyVM vm;
    tsfi_strategy_vm_init(&vm);

    TSFiPriorityQueue pq;
    tsfi_priority_queue_init(&pq);
    tsfi_priority_queue_push(&pq, 10, 30, "GOAL_A");
    tsfi_priority_queue_push(&pq, 5, 31, "GOAL_B");

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < ITERATIONS; i++) {
        // Reset scale factors to simulate continuous runs
        vm.depth_priority_scale = 1;
        vm.abductive_priority_scale = 1;
        res = tsfi_strategy_vm_execute_bytecode(&vm, &pq, bytecode, bc_len, NULL);
        if (res != 0) {
            fprintf(stderr, "VM execution failed at iteration %d\n", i);
            return 1;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    double latency = elapsed_ns / ITERATIONS;

    printf("  Executed %d Strategy VM cycles successfully.\n", ITERATIONS);
    printf("  Total Time: %.2f ms\n", elapsed_ns / 1e6);
    printf("  Average Latency: %.2f ns/cycle\n", latency);
    fflush(stdout);

    // Profile COBOL COMP-3 Hex-BCD packing/unpacking
    printf("[COBOL COMP-3 Benchmark] Profiling BCD packing and unpacking...\n");
    const char *evm_address = "90f79bf6eb2c4f870365e785982e1f101e93b906";
    uint8_t packed[32];
    char unpacked[64];
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < ITERATIONS; i++) {
        int packed_len = tsfi_cobol_pack_hex(evm_address, packed, sizeof(packed));
        int unpack_res = tsfi_cobol_unpack_hex(packed, packed_len, unpacked, sizeof(unpacked));
        (void)unpack_res;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    elapsed_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    double bcd_latency = elapsed_ns / ITERATIONS;
    printf("  Packed/Unpacked %d addresses successfully.\n", ITERATIONS);
    printf("  Total BCD Time: %.2f ms\n", elapsed_ns / 1e6);
    printf("  Average BCD Latency: %.2f ns/op\n", bcd_latency);
    fflush(stdout);

    return 0;
}
