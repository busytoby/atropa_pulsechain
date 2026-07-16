#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_unified_classifier.h"
#include "tsfi_algebraic_query.h"

#define ITERATIONS 100000

int main(void) {
    printf("[Benchmark] Starting high-precision unified classifier benchmarks (%d iterations)...\n", ITERATIONS);
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    // Setup 10 coordinates to simulate a dense subgrid
    for (int i = 0; i < 10; i++) {
        char key[128];
        snprintf(key, sizeof(key), "svdag/%d/10/10", i);
        tsfi_trie_insert(trie_root, key, (i % 2 == 0) ? "RELATION_TRUE" : "QUESTION_PENDING");
    }

    // Setup priors
    tsfi_trie_insert(trie_root, "prior/holds_token", "60");
    tsfi_trie_insert(trie_root, "prior/active", "50");
    tsfi_trie_insert(trie_root, "query/joint/prior/holds_token/prior/active", "40");

    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    // Setup algebraic query
    TSFiQueryNode *var_a = tsfi_query_create_var("prior/holds_token");
    TSFiQueryNode *var_b = tsfi_query_create_var("prior/active");
    TSFiQueryNode *query_tree = tsfi_query_create_mul(var_a, var_b);

    struct timespec start, end;
    double elapsed_ns;

    // 1. Benchmark: Algebraic Query Evaluation
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < ITERATIONS; i++) {
        volatile float res = tsfi_query_evaluate(query_tree, dat);
        (void)res;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = (double)(end.tv_sec - start.tv_sec) * 1e9 + (double)(end.tv_nsec - start.tv_nsec);
    printf("  [BENCHMARK] Algebraic Query Evaluation: %.2f ns/op\n", elapsed_ns / ITERATIONS);
    fflush(stdout);

    // 2. Benchmark: Bayesian Posterior Inference
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < ITERATIONS; i++) {
        volatile float res = tsfi_bayesian_infer(dat, "prior/holds_token", "prior/active");
        (void)res;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = (double)(end.tv_sec - start.tv_sec) * 1e9 + (double)(end.tv_nsec - start.tv_nsec);
    printf("  [BENCHMARK] Bayesian Posterior Inference: %.2f ns/op\n", elapsed_ns / ITERATIONS);
    fflush(stdout);

    // 3. Benchmark: Unified Region Classification (10-coordinate scan + Bayesian)
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < ITERATIONS; i++) {
        volatile TSFiUnifiedVoxelState res = tsfi_unified_classify_region(dat, 0, 9, 10, 10, 
                                                                          "prior/holds_token", "prior/active");
        (void)res.density_class;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_ns = (double)(end.tv_sec - start.tv_sec) * 1e9 + (double)(end.tv_nsec - start.tv_nsec);
    printf("  [BENCHMARK] Unified Region Classification Scan: %.2f ns/op\n", elapsed_ns / ITERATIONS);
    fflush(stdout);

    // Cleanup
    tsfi_query_destroy(query_tree);
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    return 0;
}
