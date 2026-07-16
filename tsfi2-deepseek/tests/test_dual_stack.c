#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_dual_stack.h"

int main(void) {
    printf("[Dual Stack] Starting semantic-constraint dual stack VM tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiAKB *akb = tsfi_akb_create(dat);
    assert(akb != NULL);

    // 1. Program: Successful execution
    // - PUSH_CONS 1 (Valid constraint)
    // - VERIFY_CONS
    // - PUSH_SEMA 1 (SVDAG coordinate index)
    // - RESOLVE_SEMA (Deduces temporary state)
    // - HALT_DUAL
    int bytecode_success[8] = {
        OP_PUSH_CONS, 1,
        OP_VERIFY_CONS,
        OP_PUSH_SEMA, 1,
        OP_RESOLVE_SEMA,
        OP_HALT_DUAL
    };

    TSFiDualStackVM vm1;
    int res = tsfi_dual_stack_execute(&vm1, akb, bytecode_success, 8);
    printf("  [Execution Success] Backtrack Status: %d (Expected 0), Halted: %d\n", res, vm1.halted);
    fflush(stdout);
    assert(res == 0);
    assert(vm1.halted == 1);
    
    // Verify that the semantic stack successfully constructed a temporary coordinate state
    const char *val = tsfi_akb_read(akb, "svdag/1/10/10");
    printf("  [Execution Success] Deduced Cache Value: %s (Expected TEMP_RELATION_TRUE)\n", val);
    fflush(stdout);
    assert(val && strcmp(val, "TEMP_RELATION_TRUE") == 0);

    // 2. Program: Failed constraint execution
    // - PUSH_CONS 0 (Invalid constraint)
    // - VERIFY_CONS (Backtracks immediately, skips semantic resolution)
    // - PUSH_SEMA 1
    // - RESOLVE_SEMA
    // - HALT_DUAL
    int bytecode_fail[8] = {
        OP_PUSH_CONS, 0,
        OP_VERIFY_CONS,
        OP_PUSH_SEMA, 1,
        OP_RESOLVE_SEMA,
        OP_HALT_DUAL
    };

    TSFiAKB *akb2 = tsfi_akb_create(dat);
    TSFiDualStackVM vm2;
    res = tsfi_dual_stack_execute(&vm2, akb2, bytecode_fail, 8);
    printf("  [Execution Fail] Backtrack Status: %d (Expected 1), Halted: %d\n", res, vm2.halted);
    fflush(stdout);
    assert(res == 1); // Backtracked
    assert(vm2.halted == 0); // Did not reach halt

    // Verify no temporary coordinates were constructed due to early backtrack
    const char *val_fail = tsfi_akb_read(akb2, "svdag/1/10/10");
    assert(val_fail == NULL);

    // Cleanup
    tsfi_akb_destroy(akb);
    tsfi_akb_destroy(akb2);
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Semantic-constraint dual stack VM verified successfully!\n");
    fflush(stdout);
    return 0;
}
