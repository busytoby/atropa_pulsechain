#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_unified_classifier.h"

int main(void) {
    printf("[Unified Classifier] Starting unified voxel classification pipeline tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);

    // Setup SVDAG coordinate range: x=0 is RELATION_TRUE, x=1 is QUESTION_PENDING
    tsfi_trie_insert(trie_root, "svdag/0/10/10", "RELATION_TRUE");
    tsfi_trie_insert(trie_root, "svdag/1/10/10", "QUESTION_PENDING");

    // Setup Bayesian priors
    tsfi_trie_insert(trie_root, "prior/holds_token", "60"); // P(H) = 60%
    tsfi_trie_insert(trie_root, "prior/active", "50");      // P(E) = 50%
    tsfi_trie_insert(trie_root, "query/joint/prior/holds_token/prior/active", "40");

    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    // Run unified pipeline over x=[0..1], y=10, z=10
    TSFiUnifiedVoxelState state = tsfi_unified_classify_region(dat, 0, 1, 10, 10,
                                                               "prior/holds_token", "prior/active");

    printf("  [Unified State] Density Class: %d (Expected CLASS_BOND=2 or CLASS_SEED=1)\n", state.density_class);
    printf("  [Unified State] Strain Class: %d (Expected CLASS_FRONTIER=11)\n", state.strain_class);
    printf("  [Unified State] Confidence: %.2f, Strain Gradient: %.2f\n", state.confidence, state.strain_gradient);
    printf("  [Unified State] Posterior Probability: %.2f (Expected 0.80)\n", state.posterior_probability);
    fflush(stdout);

    // Asserts
    assert(state.density_class == CLASS_BOND || state.density_class == CLASS_SEED);
    assert(state.strain_class == CLASS_FRONTIER);
    assert(state.strain_gradient > 0.40f);
    assert(fabs(state.posterior_probability - 0.80f) < 0.0001f);

    // Cleanup
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Unified voxel state classification verified successfully!\n");
    fflush(stdout);
    return 0;
}
