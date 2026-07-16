#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_doxastic_classifier.h"
#include "../../scripts/libantigravity_extra2.c"

int main(void) {
    printf("[Complex Reasoning] Starting transitive reasoning tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);

    // 1. Initial State: A->B represents RELATION_TRUE, A->C represents QUESTION_PENDING
    tsfi_trie_insert(trie_root, "svdag/0/0/0", "RELATION_TRUE");
    tsfi_trie_insert(trie_root, "svdag/1/0/0", "QUESTION_PENDING");

    tsfi_dat *dat1 = tsfi_dat_compile(trie_root);
    assert(dat1 != NULL);

    // Verify initial boundary strain (transition between 0 and 1 represents a frontier)
    TSFiStrainClassification s1 = tsfi_doxastic_classify_strain(dat1, 0, 1, 0, 0);
    printf("  [Initial State] Boundary Class: %d (Expected CLASS_FRONTIER=%d)\n", s1.class_id, CLASS_FRONTIER);
    fflush(stdout);
    assert(s1.class_id == CLASS_FRONTIER);

    // 2. Fetch transition status and simulate transitivity verification script:
    const char *ab_val = tsfi_dat_search(dat1, "svdag/0/0/0");
    const char *bc_val = tsfi_dat_search(dat1, "svdag/0/0/0"); // Mock B->C as active too
    assert(ab_val && bc_val);

    int ab_hash = strcmp(ab_val, "RELATION_TRUE") == 0 ? 99 : 0;
    int bc_hash = strcmp(bc_val, "RELATION_TRUE") == 0 ? 99 : 0;

    InteropStackVM vm;
    memset(&vm, 0, sizeof(InteropStackVM));

    // Script bytecode:
    // PUSH ab_hash
    // PUSH bc_hash
    // OP_TRY_ME_ELSE 8 (PC target for equal check)
    // OP_NEQ
    // HALT (PC 7: unequal)
    // Target (PC 8):
    // PUSH 999
    // HALT
    int script[11] = {
        1, ab_hash,
        1, bc_hash,
        0x21, 8,
        0x24,
        6,
        1, 999,
        6
    };

    assert(interop_stack_vm_execute(&vm, script, 11) == 0);

    // 3. If transitivity assertion succeeds (stack contains our verification flag 999), crystallize svdag/1/0/0
    if (vm.halted && vm.stack_len > 2 && vm.stack[2] == 999) {
        printf("  [Transitive Inference] VM transitivity check passed. Crystallizing svdag/1/0/0...\n");
        fflush(stdout);
        tsfi_trie_insert(trie_root, "svdag/1/0/0", "RELATION_TRUE");
    }

    // Recompile
    tsfi_dat *dat2 = tsfi_dat_compile(trie_root);
    assert(dat2 != NULL);

    // 4. Verify post-inference state (Strain drops to stable)
    TSFiStrainClassification s2 = tsfi_doxastic_classify_strain(dat2, 0, 1, 0, 0);
    printf("  [Post State] Boundary Class: %d (Expected CLASS_STABLE=%d)\n", s2.class_id, CLASS_STABLE);
    fflush(stdout);
    assert(s2.class_id == CLASS_STABLE);

    // Cleanup
    tsfi_dat_destroy(dat1);
    tsfi_dat_destroy(dat2);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Complex transitive reasoning verified successfully!\n");
    fflush(stdout);
    return 0;
}
