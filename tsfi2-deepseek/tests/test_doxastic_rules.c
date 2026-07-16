#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_doxastic_classifier.h"
#include "../../scripts/libantigravity_extra2.c"

int main(void) {
    printf("[Rules Engine] Starting doxastic rules crystallization test...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    
    // 1. Initial State: Region [0..2] consists of pending questions (Potential Front)
    tsfi_trie_insert(trie_root, "svdag/0/10/10", "QUESTION_PENDING");
    tsfi_trie_insert(trie_root, "svdag/1/10/10", "QUESTION_PENDING");
    tsfi_trie_insert(trie_root, "svdag/2/10/10", "QUESTION_PENDING");

    tsfi_dat *dat1 = tsfi_dat_compile(trie_root);
    assert(dat1 != NULL);

    // Verify initial classification is CLASS_SEED
    TSFiDoxasticClassification c1 = tsfi_doxastic_classify_region(dat1, 0, 2, 10, 10);
    printf("  [Initial State] Class: %d (Expected CLASS_SEED=%d), Confidence: %.2f\n", 
           c1.class_id, CLASS_SEED, c1.confidence);
    fflush(stdout);
    assert(c1.class_id == CLASS_SEED);

    // 2. VM Script solves the open question
    InteropStackVM vm;
    memset(&vm, 0, sizeof(InteropStackVM));
    int solve_script[6] = {
        1, 100,
        1, 200,
        0x24, // OP_NEQ (succeeds, 100 != 200)
        6
    };
    assert(interop_stack_vm_execute(&vm, solve_script, 6) == 0);
    
    // Crystallize the answer on success
    if (vm.halted) {
        printf("  [Execution] VM resolved constraint successfully. Crystallizing facts...\n");
        fflush(stdout);
        // Overwrite pending questions with verified RELATION_TRUE
        tsfi_dat_compile_relation(trie_root, "svdag/0", "10", "10");
        tsfi_dat_compile_relation(trie_root, "svdag/1", "10", "10");
        tsfi_dat_compile_relation(trie_root, "svdag/2", "10", "10");
    }

    // Recompile trie
    tsfi_dat *dat2 = tsfi_dat_compile(trie_root);
    assert(dat2 != NULL);

    // 3. Post State: Region [0..2] is now crystallized facts (CLASS_BOND)
    TSFiDoxasticClassification c2 = tsfi_doxastic_classify_region(dat2, 0, 2, 10, 10);
    printf("  [Post State] Class: %d (Expected CLASS_BOND=%d), Confidence: %.2f\n", 
           c2.class_id, CLASS_BOND, c2.confidence);
    fflush(stdout);
    assert(c2.class_id == CLASS_BOND);

    // Cleanup
    tsfi_dat_destroy(dat1);
    tsfi_dat_destroy(dat2);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Doxastic rules crystallization test completed successfully!\n");
    fflush(stdout);
    return 0;
}
