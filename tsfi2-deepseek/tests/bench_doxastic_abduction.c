#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "../../scripts/libantigravity_extra2.c"

#define ITERATIONS 100000

int main(void) {
    printf("[Benchmark] Starting High-Precision Doxastic Abduction Benchmarks...\n");
    printf("[Benchmark] Running %d iterations per target...\n", ITERATIONS);
    fflush(stdout);

    // Setup dummy VM
    InteropStackVM vm;
    int script[16] = {
        1, -9999,
        1, 20,
        0x26, 13,
        1, 101,
        1, 0,
        0x25,
        0x22,
        6,
        1, 200,
        6
    };

    // 1. Benchmark Stochastic Choice Rollouts and Backtracking
    struct timespec s1, e1;
    clock_gettime(CLOCK_MONOTONIC, &s1);
    
    for (int i = 0; i < ITERATIONS; i++) {
        memset(&vm, 0, sizeof(InteropStackVM));
        int res = interop_stack_vm_execute(&vm, script, 15);
        (void)res;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &e1);
    long long t1_ns = (e1.tv_sec - s1.tv_sec) * 1000000000LL + (e1.tv_nsec - s1.tv_nsec);
    double t1_avg = (double)t1_ns / ITERATIONS;
    double t1_ops = 1000000000.0 / t1_avg;
    
    printf("  [Benchmark] Stochastic VM Rollout Latency: %.2f ns/op (%.2f Million ops/sec)\n", 
           t1_avg, t1_ops / 1000000.0);
    fflush(stdout);

    // 2. Benchmark Trie Compilations and Retractions
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    struct timespec s2, e2;
    clock_gettime(CLOCK_MONOTONIC, &s2);
    
    // We run 10,000 iterations for compiling because compiling is heavier than stack VM execution
    int compile_iter = ITERATIONS / 10; 
    for (int i = 0; i < compile_iter; i++) {
        tsfi_dat *dat = tsfi_dat_compile_relation(trie_root, "bench", "node", "1");
        tsfi_dat_retract_relation(trie_root, "bench", "node", "1");
        tsfi_dat_destroy(dat);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &e2);
    long long t2_ns = (e2.tv_sec - s2.tv_sec) * 1000000000LL + (e2.tv_nsec - s2.tv_nsec);
    double t2_avg = (double)t2_ns / compile_iter;
    double t2_ops = 1000000000.0 / t2_avg;
    
    printf("  [Benchmark] DAT Relation Compile + Retract Latency: %.2f ns/op (%.2f ops/sec)\n", 
           t2_avg, t2_ops);
    fflush(stdout);

    tsfi_trie_destroy(trie_root);
    printf("[Benchmark] Benchmarks completed successfully!\n");
    fflush(stdout);
    return 0;
}
