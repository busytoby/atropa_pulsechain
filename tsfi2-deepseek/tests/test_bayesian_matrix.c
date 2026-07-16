#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_bayesian.h"

int main(void) {
    printf("[Bayesian Matrix] Starting Bayesian network and matrix operator tests...\n");
    fflush(stdout);

    // 1. Verify Transition Matrix Powers (nilpotent reachability checks)
    // Create a 3-node transition matrix: State 0 -> State 1 (50%), State 1 -> State 2 (80%)
    TSFiTransitionMatrix T = {
        .size = 3,
        .data = {
            { 0.0f, 0.5f, 0.0f },
            { 0.0f, 0.0f, 0.8f },
            { 0.0f, 0.0f, 0.0f }
        }
    };

    TSFiTransitionMatrix T2;
    tsfi_matrix_power(&T, 2, &T2);

    // T^2 element [0][2] represents P(0 -> 2 in 2 steps) = 0.5 * 0.8 = 0.40 (40%)
    printf("  [Matrix] T^2[0][2]: %.2f (Expected 0.40)\n", T2.data[0][2]);
    fflush(stdout);
    assert(fabs(T2.data[0][2] - 0.40f) < 0.0001f);

    // T^3 should be zero matrix since DAG size is 3 (nilpotent index 3)
    TSFiTransitionMatrix T3;
    tsfi_matrix_power(&T, 3, &T3);
    printf("  [Matrix] T^3[0][2]: %.2f (Expected 0.00)\n", T3.data[0][2]);
    fflush(stdout);
    assert(fabs(T3.data[0][2] - 0.00f) < 0.0001f);

    // 2. Verify Bayesian Inference
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    // Setup priors
    tsfi_trie_insert(trie_root, "prior/holds_token", "60"); // P(H) = 60%
    tsfi_trie_insert(trie_root, "prior/active", "50");      // P(E) = 50%
    // Setup explicit joint P(H AND E) = 40%
    tsfi_trie_insert(trie_root, "query/joint/prior/holds_token/prior/active", "40");

    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    // Compute posterior: P(H | E) = P(H AND E) / P(E) = 0.40 / 0.50 = 0.80 (80%)
    float posterior = tsfi_bayesian_infer(dat, "prior/holds_token", "prior/active");
    printf("  [Inference] P(H | E): %.2f (Expected 0.80)\n", posterior);
    fflush(stdout);
    assert(fabs(posterior - 0.80f) < 0.0001f);

    // Cleanup
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Bayesian network and matrix operators verified successfully!\n");
    fflush(stdout);
    return 0;
}
