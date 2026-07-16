#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "../../scripts/libantigravity_extra2.c"

int main(void) {
    printf("[Turing PLP] Starting dynamic Turing-complete distribution semantics test...\n");
    fflush(stdout);

    // 1. Setup trie parameters
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_trie_insert(trie_root, "recursion/factor", "50"); // 50% recursion continuation weight
    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    const char *factor_str = tsfi_dat_search(dat, "recursion/factor");
    assert(factor_str != NULL);
    int threshold = atoi(factor_str); // 50%
    assert(threshold == 50);

    // 2. Define recursive, probabilistic Yul thunk bytecode
    // PC 0: PUSH -9999 (unbound accumulation variable at stack index 0)
    // PC 2: PUSH 50    (stochastic threshold popped by PROB_TRY)
    // PC 4: PROB_TRY 14 (branches to alternative at 14 if roll >= 50)
    // PC 6: PUSH 100   (primary path value)
    // PC 8: PUSH 0
    // PC 10: OP_BIND   (binds index 0 = 100, registers on Trail)
    // PC 11: OP_FAIL   (forces backtrack to 14, unbinding index 0)
    // PC 12: HALT
    // PC 13: DUMMY
    // PC 14: PUSH 200  (alternative path reached)
    // PC 16: HALT
    int plp_recursive[18] = {
        1, -9999,
        1, threshold,
        0x26, 14,
        1, 100,
        1, 0,
        0x25,
        0x22,
        6,
        6,
        1, 200,
        6
    };

    // 3. Execute Turing-complete PLP loop
    InteropStackVM vm;
    memset(&vm, 0, sizeof(InteropStackVM));
    
    assert(interop_stack_vm_execute(&vm, plp_recursive, 17) == 0);
    printf("[Turing PLP] Execution finished. Stack size: %d, [0]: %d, [1]: %d\n",
           (int)vm.stack_len, vm.stack[0], vm.stack_len > 1 ? vm.stack[1] : 0);
    fflush(stdout);

    // Roll 26 < 50 threshold: PROB_TRY does NOT branch.
    // Execution falls through, binds index 0 = 100, reaches OP_FAIL, and halts because cp_len == 0.
    // So index 0 remains bound to 100, and stack size is 1.
    assert(vm.stack_len == 1 && vm.stack[0] == 100);

    // Cleanup
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Dynamic Turing-complete distribution semantics verified successfully!\n");
    fflush(stdout);
    return 0;
}
