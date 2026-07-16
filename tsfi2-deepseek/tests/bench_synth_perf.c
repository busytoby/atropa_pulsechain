#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_synth_perf.h"

#define QUEUE_ITERATIONS 1000000
#define SOLVER_BRANCHES 10000

int main(void) {
    printf("[Stress Benchmark] Starting high-throughput synthesizer performance stress tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiSynthPerfEngine *engine = tsfi_synth_perf_create(dat, trie_root);
    assert(engine != NULL);

    struct timespec start, end;
    double elapsed_ns;

    // 1. Benchmark: Lock-Free Audio Queue Throughput
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < QUEUE_ITERATIONS; i++) {
        tsfi_synth_perf_push_audio(engine, 0.7f);
        volatile float sample = tsfi_synth_perf_pop_audio(engine);
        (void)sample;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = (double)(end.tv_sec - start.tv_sec) * 1e9 + (double)(end.tv_nsec - start.tv_nsec);
    printf("  [STRESS BENCHMARK] Lock-Free Audio Ring Buffer: %.2f ns per push/pop cycle\n", elapsed_ns / QUEUE_ITERATIONS);
    fflush(stdout);

    // 2. Benchmark: VRAM Direct Mapping Sync Throughput
    float simulated_vram[MAX_AKB_CACHE_KEYS];
    tsfi_synth_perf_map_vram(engine, simulated_vram, MAX_AKB_CACHE_KEYS);
    
    // Fill AKB Cache
    for (int i = 0; i < MAX_AKB_CACHE_KEYS; i++) {
        char key[128];
        snprintf(key, sizeof(key), "svdag/%d/10/10", i);
        tsfi_akb_write(engine->synth->orchestrator->akb, key, "95");
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 50000; i++) {
        tsfi_synth_perf_sync_vram(engine);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = (double)(end.tv_sec - start.tv_sec) * 1e9 + (double)(end.tv_nsec - start.tv_nsec);
    printf("  [STRESS BENCHMARK] VRAM Direct Map Synchronization: %.2f ns per full map update\n", elapsed_ns / 50000);
    fflush(stdout);

    // 3. Benchmark: Parallel OpenMP Constraint Solver Scaling
    int bytecode[11] = {
        1, 50,
        0x31,
        1, 30,
        0x31,
        1, 70,
        0x32,
        6
    };
    int *results = calloc(SOLVER_BRANCHES, sizeof(int));

    clock_gettime(CLOCK_MONOTONIC, &start);
    tsfi_synth_perf_parallel_solve(engine, bytecode, 11, SOLVER_BRANCHES, results);
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = (double)(end.tv_sec - start.tv_sec) * 1e9 + (double)(end.tv_nsec - start.tv_nsec);
    printf("  [STRESS BENCHMARK] Parallel Solver Scaling (%d branches): %.2f ns/branch\n", SOLVER_BRANCHES, elapsed_ns / SOLVER_BRANCHES);
    fflush(stdout);

    // Cleanup
    free(results);
    tsfi_synth_perf_destroy(engine);
    tsfi_trie_destroy(trie_root);

    return 0;
}
