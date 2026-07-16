#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "../../scripts/libantigravity_extra2.c"

int main(void) {
    printf("[Disjunction] Starting disjunctive backtracking tests...\n");
    fflush(stdout);

    // 1. Script representing disjunction: (A OR B) -> C
    // Target PC for B is 11
    int disj_script[17] = {
        1, -9999,    // 0: PUSH -9999 (Variable)
        0x21, 11,    // 2: TRY_ME_ELSE to PC 11 (Branch B)
        1, 50,       // 4: PUSH 50
        1, 50,       // 6: PUSH 50 (to force failure)
        0x24,        // 8: OP_NEQ (fails because 50 == 50, triggers backtrack to 11)
        6,           // 9: HALT (not reached)
        6,           // 10: DUMMY
        1, 999,      // 11: PUSH 999 (Branch B)
        1, 0,        // 13: PUSH 0
        0x25,        // 15: OP_BIND (binds stack[0] = 999)
        6            // 16: HALT
    };

    // 2. Execute VM
    InteropStackVM vm;
    memset(&vm, 0, sizeof(InteropStackVM));
    
    assert(interop_stack_vm_execute(&vm, disj_script, 17) == 0);
    printf("[Disjunction] Execution finished. Stack size: %d, [0]: %d\n", 
           (int)vm.stack_len, vm.stack[0]);
    fflush(stdout);

    // Assert that branch A failed, and branch B was successfully evaluated and bound
    assert(vm.stack_len == 1 && vm.stack[0] == 999);

    printf("[PASS] Disjunctive choice-point backtracking verified successfully!\n");
    fflush(stdout);
    return 0;
}
