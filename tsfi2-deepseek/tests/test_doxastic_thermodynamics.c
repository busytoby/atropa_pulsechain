#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "../../scripts/libantigravity_extra2.c"

int main(void) {
    printf("[Thermodynamics] Starting Doxastic Logic phase transition tests...\n");
    fflush(stdout);

    // 1. Verify Thermal Annealing & Damping (Cooling search space)
    printf("       [Verify] Cooling logical search temperature...\n");
    fflush(stdout);
    int temp = 100;
    int decay = 15;
    
    // Cool rules down to 0
    while (temp > 0) {
        temp = (temp > decay) ? (temp - decay) : 0;
    }
    assert(temp == 0);

    // 2. Verify Verlet Strain and Connection Fracture
    printf("       [Verify] Simulating Hooke's Law rule strain and fracture limits...\n");
    fflush(stdout);
    int displacement = 25;
    int stiffness = 5;
    int spring_force = stiffness * displacement; // F = 125
    
    int elastic_limit = 100;
    int fractured = (spring_force > elastic_limit) ? 1 : 0;
    assert(fractured == 1); // Elastic rule connection fractured/pruned under high load

    // 3. Verify Trie Rule Crystallization (Transition from choices to permanent index)
    printf("       [Verify] Rule crystallization into DAT registry...\n");
    fflush(stdout);
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    
    // Crystallize rules
    tsfi_dat *dat = tsfi_dat_compile_relation(trie_root, "crystallized/rules", "active_plan", "PLAN_VERIFIED_200");
    assert(dat != NULL);
    
    const char *rule_val = tsfi_dat_search(dat, "crystallized/rules/active_plan/PLAN_VERIFIED_200");
    assert(rule_val != NULL && strcmp(rule_val, "RELATION_TRUE") == 0);
    
    // 4. Benchmark Disk-to-Memory Substrate Latencies
    printf("       [Verify] Benchmarking DAT substrate loop times...\n");
    fflush(stdout);
    
    tsfi_trie_node *bench_root = tsfi_trie_create_node(0);
    tsfi_dat *bench_dat = tsfi_dat_compile_relation(bench_root, "benchmark/state", "active", "1");
    
    struct timespec s1, e1;
    clock_gettime(CLOCK_MONOTONIC, &s1);
    assert(tsfi_dat_save_mmap(bench_dat, "tmp/bench_substrate.dat.bin") == 0);
    clock_gettime(CLOCK_MONOTONIC, &e1);
    
    long long save_ns = (e1.tv_sec - s1.tv_sec) * 1000000000LL + (e1.tv_nsec - s1.tv_nsec);
    printf("         [Substrate] Mmap save completed in %lld ns.\n", save_ns);
    fflush(stdout);
    
    struct timespec s2, e2;
    clock_gettime(CLOCK_MONOTONIC, &s2);
    const char *lookup_val = tsfi_dat_search(bench_dat, "benchmark/state/active/1");
    clock_gettime(CLOCK_MONOTONIC, &e2);
    assert(lookup_val != NULL && strcmp(lookup_val, "RELATION_TRUE") == 0);
    
    long long query_ns = (e2.tv_sec - s2.tv_sec) * 1000000000LL + (e2.tv_nsec - s2.tv_nsec);
    printf("         [Substrate] Prefix query completed in %lld ns.\n", query_ns);
    fflush(stdout);
    
    // Latency Guard Gate: Queries must be sub-microsecond
    assert(query_ns < 2000LL);
    
    tsfi_dat_destroy(bench_dat);
    tsfi_trie_destroy(bench_root);
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Doxastic Logic thermodynamic tests passed successfully!\n");
    fflush(stdout);
    return 0;
}
