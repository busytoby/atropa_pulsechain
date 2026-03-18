#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tsfi_nand_trap_firmware.h"
#include "tsfi_controlnet_shm.h"
#include "tsfi_wire_firmware.h"
#include "lau_memory.h"
#include "tsfi_wavelet_arena.h"

#define BENCH_ITERATIONS 1000000

static double get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

int main() {
    printf("=== TSFi Benchmark: ZMM XNOR Trap (Equivalence Boundary) ===\n");

    tsfi_wire_firmware_init();
    void *manifold = calloc(1, TSFI_CN_MAP_SIZE);
    
    TsfiWavelet W_Genie = {0};
    TsfiWavelet W_SVDAG = {0};

    // Load with arbitrary matching data to ensure positive solidification
    memset(W_Genie.payload, 0x42, 120);
    memset(W_SVDAG.payload, 0x42, 120);

    // Warmup
    for (int i = 0; i < 1000; i++) {
        tsfi_wavelet_xnor_signal(manifold, &W_Genie, &W_SVDAG);
    }

    // Benchmark matched isomorphism (Rigid State)
    double start_ns = get_time_ns();
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        tsfi_wavelet_xnor_signal(manifold, &W_Genie, &W_SVDAG);
    }
    double end_ns = get_time_ns();
    
    double elapsed_ns = end_ns - start_ns;
    double ns_per_trap = elapsed_ns / BENCH_ITERATIONS;
    double ops_per_sec = (1e9 / ns_per_trap) / 1e6;

    printf("\n[RESULTS] Isomorphism State (Matched):\n");
    printf("Iterations: %d\n", BENCH_ITERATIONS);
    printf("Total Time: %.2f ms\n", elapsed_ns / 1e6);
    printf("Latency:    %.2f ns per XNOR Trap\n", ns_per_trap);
    printf("Throughput: %.2f Million Traps / sec\n", ops_per_sec);

    // Introduce a fracture
    W_SVDAG.payload[55] = 0x99;

    start_ns = get_time_ns();
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        tsfi_wavelet_xnor_signal(manifold, &W_Genie, &W_SVDAG);
    }
    end_ns = get_time_ns();
    
    elapsed_ns = end_ns - start_ns;
    ns_per_trap = elapsed_ns / BENCH_ITERATIONS;
    ops_per_sec = (1e9 / ns_per_trap) / 1e6;

    printf("\n[RESULTS] Fracture State (Mismatched):\n");
    printf("Iterations: %d\n", BENCH_ITERATIONS);
    printf("Total Time: %.2f ms\n", elapsed_ns / 1e6);
    printf("Latency:    %.2f ns per XNOR Trap\n", ns_per_trap);
    printf("Throughput: %.2f Million Traps / sec\n", ops_per_sec);

    free(manifold);
    printf("\n[ABSOLUTE SUCCESS] XNOR Trap Benchmarks Complete.\n");
    return 0;
}
