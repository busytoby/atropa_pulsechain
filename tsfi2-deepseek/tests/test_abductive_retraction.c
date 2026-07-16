#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "../../scripts/libantigravity_extra2.c"

int main(void) {
    printf("[Abductive Retraction] Starting closed-loop feedback tests...\n");
    fflush(stdout);

    // 1. Setup initial assumed hypothesis in trie
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_dat *dat1 = tsfi_dat_compile_relation(trie_root, "abduce", "hypothesis_a", "1");
    assert(dat1 != NULL);
    
    // Verify it exists in database
    const char *h1 = tsfi_dat_search(dat1, "abduce/hypothesis_a/1");
    assert(h1 != NULL && strcmp(h1, "RELATION_TRUE") == 0);
    printf("  [Step 1] Assumed hypothesis_a is true.\n");
    fflush(stdout);

    // 2. Evaluate constraints using VM (Forces failure)
    InteropStackVM vm;
    memset(&vm, 0, sizeof(InteropStackVM));
    
    // Script pushes two equal values and runs OP_NEQ, forcing constraint failure
    int constraint_script[6] = {
        1, 100,
        1, 100,
        0x24, // OP_NEQ (fails because 100 == 100, halts VM)
        6
    };
    
    assert(interop_stack_vm_execute(&vm, constraint_script, 6) == 0);
    printf("  [Step 2] Constraint evaluated. VM halted status: %d\n", vm.halted);
    fflush(stdout);
    
    // 3. Closed-Loop Retraction: If VM halted due to constraint failure, retract the hypothesis!
    if (vm.halted) {
        printf("  [Step 3] Constraint failed! Retracting hypothesis_a from DAT rails...\n");
        fflush(stdout);
        assert(tsfi_dat_retract_relation(trie_root, "abduce", "hypothesis_a", "1") == 0);
    }
    
    // 4. Re-compile and verify deletion
    tsfi_dat *dat2 = tsfi_dat_compile(trie_root);
    assert(dat2 != NULL);
    
    const char *h2 = tsfi_dat_search(dat2, "abduce/hypothesis_a/1");
    assert(h2 == NULL || strcmp(h2, "") == 0);
    printf("  [Step 4] Verified hypothesis_a has been wiped from memory-mapped database.\n");
    fflush(stdout);

    // Cleanup
    tsfi_dat_destroy(dat1);
    tsfi_dat_destroy(dat2);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Closed-loop abductive retraction loop verified successfully!\n");
    fflush(stdout);
    return 0;
}
