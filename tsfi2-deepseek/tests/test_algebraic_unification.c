#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "../../scripts/libantigravity_extra2.c"

int main(void) {
    printf("[Algebraic Unification] Starting algebraic solver test...\n");
    fflush(stdout);

    // 1. Verify Verlet solver integration
    int next_x = tsfi_vaesen_solve_step(100, 90, 5);
    assert(next_x == 115);
    printf("[Algebraic Unification] Verlet step matches: %d\n", next_x);
    fflush(stdout);

    // 2. Verify polynomial evaluating relation compiler
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    
    // Evaluate quadratic polynomial y = 2*x^2 + 3*x + 5 for x = 10 (y = 235)
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    tsfi_dat *poly_dat = tsfi_dat_compile_relation(trie_root, "poly_eval/quadratic", "x_10", "y_235");
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    assert(poly_dat != NULL);
    
    long long duration_ns = (end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
    printf("[Algebraic Unification] Polynomial relation compiled in %lld ns.\n", duration_ns);
    fflush(stdout);
    
    // Validate search
    const char *res = tsfi_dat_search(poly_dat, "poly_eval/quadratic/x_10/y_235");
    assert(res != NULL && strcmp(res, "RELATION_TRUE") == 0);
    
    tsfi_dat_destroy(poly_dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Algebraic evaluation verified successfully!\n");
    fflush(stdout);
    return 0;
}
