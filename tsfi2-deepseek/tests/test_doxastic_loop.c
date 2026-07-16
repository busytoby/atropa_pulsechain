#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_doxastic_classifier.h"
#include "../../scripts/libantigravity_extra2.c"

int main(void) {
    printf("[Self-Refining Loop] Starting closed-loop stabilization tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);

    // 1. Initial State: Setup a high-potential learning front (CLASS_FRONTIER)
    // x=0 is a solid fact. x=1 is a pending question.
    tsfi_trie_insert(trie_root, "svdag/0/15/15", "RELATION_TRUE");
    tsfi_trie_insert(trie_root, "svdag/1/15/15", "QUESTION_PENDING");

    tsfi_dat *dat1 = tsfi_dat_compile(trie_root);
    assert(dat1 != NULL);

    // Verify initial state is classified as FRONTIER boundary
    TSFiStrainClassification s1 = tsfi_doxastic_classify_strain(dat1, 0, 1, 15, 15);
    printf("  [Initial State] Boundary Class: %d (Expected CLASS_FRONTIER=%d), Gradient: %.2f\n",
           s1.class_id, CLASS_FRONTIER, s1.boundary_gradient);
    fflush(stdout);
    assert(s1.class_id == CLASS_FRONTIER);

    // 2. Abduction & VM Constraint Check: Solve the frontier question
    InteropStackVM vm;
    memset(&vm, 0, sizeof(InteropStackVM));
    int query_script[6] = {
        1, 5,
        1, 10,
        0x24, // OP_NEQ (succeeds, 5 != 10)
        6
    };
    assert(interop_stack_vm_execute(&vm, query_script, 6) == 0);

    // 3. Crystallize Solution
    if (vm.halted) {
        printf("  [Abduction] Constraint satisfied! Crystallizing svdag/1/15/15...\n");
        fflush(stdout);
        tsfi_dat_compile_relation(trie_root, "svdag/1", "15", "15");
    }

    // Recompile
    tsfi_dat *dat2 = tsfi_dat_compile(trie_root);
    assert(dat2 != NULL);

    // 4. Verify Stabilization (Strain drops, class shifts to CLASS_STABLE)
    TSFiStrainClassification s2 = tsfi_doxastic_classify_strain(dat2, 0, 1, 15, 15);
    printf("  [Post State] Boundary Class: %d (Expected CLASS_STABLE=%d), Gradient: %.2f\n",
           s2.class_id, CLASS_STABLE, s2.boundary_gradient);
    fflush(stdout);
    assert(s2.class_id == CLASS_STABLE);

    // Cleanup
    tsfi_dat_destroy(dat1);
    tsfi_dat_destroy(dat2);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Closed-loop self-refining logic stabilization verified successfully!\n");
    fflush(stdout);
    return 0;
}
