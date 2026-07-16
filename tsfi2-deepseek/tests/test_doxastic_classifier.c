#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_doxastic_classifier.h"

int main(void) {
    printf("[Classifier] Starting doxastic region classifier tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);

    // 1. Setup a region with mostly crystallized facts (CLASS_BOND)
    // Target coordinate: y = 20, z = 30. X spans from 0 to 5.
    // Set X=0, 1, 2, 3 as RELATION_TRUE.
    tsfi_trie_insert(trie_root, "svdag/0/20/30", "RELATION_TRUE");
    tsfi_trie_insert(trie_root, "svdag/1/20/30", "RELATION_TRUE");
    tsfi_trie_insert(trie_root, "svdag/2/20/30", "RELATION_TRUE");
    tsfi_trie_insert(trie_root, "svdag/3/20/30", "RELATION_TRUE");
    tsfi_trie_insert(trie_root, "svdag/4/20/30", "QUESTION_PENDING");
    tsfi_trie_insert(trie_root, "svdag/5/20/30", ""); // Retracted

    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    // Classify region [0..5]
    TSFiDoxasticClassification c1 = tsfi_doxastic_classify_region(dat, 0, 5, 20, 30);
    printf("  [CLASS_BOND] Class output: %d, Confidence: %.2f\n", c1.class_id, c1.confidence);
    fflush(stdout);
    assert(c1.class_id == CLASS_BOND);
    assert(c1.confidence > 0.60f); // 4 out of 6 cells are facts

    // 2. Setup a region with mostly pending questions (CLASS_SEED)
    // Target coordinate: y = 40, z = 50. X spans from 0 to 4.
    // Set X=0, 1, 2 as QUESTION_PENDING.
    tsfi_trie_insert(trie_root, "svdag/0/40/50", "QUESTION_PENDING");
    tsfi_trie_insert(trie_root, "svdag/1/40/50", "QUESTION_PENDING");
    tsfi_trie_insert(trie_root, "svdag/2/40/50", "QUESTION_PENDING");
    tsfi_trie_insert(trie_root, "svdag/3/40/50", "RELATION_TRUE");
    tsfi_trie_insert(trie_root, "svdag/4/40/50", "");

    // Recompile trie
    tsfi_dat_destroy(dat);
    dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiDoxasticClassification c2 = tsfi_doxastic_classify_region(dat, 0, 4, 40, 50);
    printf("  [CLASS_SEED] Class output: %d, Confidence: %.2f\n", c2.class_id, c2.confidence);
    fflush(stdout);
    assert(c2.class_id == CLASS_SEED);
    assert(c2.confidence > 0.50f); // 3 out of 5 cells are pending

    // Cleanup
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Doxastic region classifier verified successfully!\n");
    fflush(stdout);
    return 0;
}
