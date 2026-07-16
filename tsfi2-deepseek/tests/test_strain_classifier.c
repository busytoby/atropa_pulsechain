#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_doxastic_classifier.h"

int main(void) {
    printf("[Strain Classifier] Starting doxastic strain boundary classifier tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);

    // 1. Setup a Frontier boundary: (Fact adjacent to Question)
    // coordinate: y = 20, z = 30. X spans from 0 to 2.
    // X=0 is RELATION_TRUE, X=1 is QUESTION_PENDING.
    tsfi_trie_insert(trie_root, "svdag/0/20/30", "RELATION_TRUE");
    tsfi_trie_insert(trie_root, "svdag/1/20/30", "QUESTION_PENDING");
    tsfi_trie_insert(trie_root, "svdag/2/20/30", "QUESTION_PENDING");

    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiStrainClassification c1 = tsfi_doxastic_classify_strain(dat, 0, 2, 20, 30);
    printf("  [CLASS_FRONTIER] Class output: %d, Gradient: %.2f\n", c1.class_id, c1.boundary_gradient);
    fflush(stdout);
    assert(c1.class_id == CLASS_FRONTIER);
    assert(c1.boundary_gradient > 0.40f); // 1 transition out of 2 is a fact-to-question swap

    // 2. Setup a Fracture boundary: (Fact adjacent to Void/empty)
    // coordinate: y = 40, z = 50. X spans from 0 to 2.
    // X=0 is RELATION_TRUE, X=1 is empty/unbound.
    tsfi_trie_insert(trie_root, "svdag/0/40/50", "RELATION_TRUE");
    tsfi_trie_insert(trie_root, "svdag/1/40/50", "");
    tsfi_trie_insert(trie_root, "svdag/2/40/50", "");

    // Recompile
    tsfi_dat_destroy(dat);
    dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiStrainClassification c2 = tsfi_doxastic_classify_strain(dat, 0, 2, 40, 50);
    printf("  [CLASS_FRACTURE] Class output: %d, Gradient: %.2f\n", c2.class_id, c2.boundary_gradient);
    fflush(stdout);
    assert(c2.class_id == CLASS_FRACTURE);
    assert(c2.boundary_gradient > 0.40f);

    // Cleanup
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Doxastic strain boundary classifier verified successfully!\n");
    fflush(stdout);
    return 0;
}
