#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_anvil_vm.h"

int main(void) {
    printf("[Anvil VM] Starting WAM Trail backtracking and subgoal memoization tests...\n");
    fflush(stdout);

    TSFiAnvilVM vm;
    tsfi_anvil_vm_init(&vm);

    // 1. Verify Variable Binding and Subgoal Creation
    tsfi_anvil_vm_bind(&vm, "svdag/relation/0", "TRUE");
    tsfi_anvil_vm_bind(&vm, "svdag/relation/1", "PENDING");

    const TSFiSubgoalEntry *entry = tsfi_anvil_vm_lookup_subgoal(&vm, "svdag/relation/0");
    printf("  [Anvil Bind] Subgoal Found: %s, State: %d (Expected SUBGOAL_CALLING=0)\n", 
           entry ? entry->key : "NULL", entry ? (int)entry->state : -1);
    fflush(stdout);
    assert(entry != NULL);
    assert(entry->state == SUBGOAL_CALLING);
    assert(vm.trail_len == 2);

    // 2. Verify Memoization State Transition
    tsfi_anvil_vm_memoize_subgoal(&vm, "svdag/relation/0", "TRUE", SUBGOAL_COMPLETED);
    entry = tsfi_anvil_vm_lookup_subgoal(&vm, "svdag/relation/0");
    printf("  [Anvil Memoize] Subgoal State: %d (Expected SUBGOAL_COMPLETED=1)\n", entry ? (int)entry->state : -1);
    fflush(stdout);
    assert(entry && entry->state == SUBGOAL_COMPLETED);

    // 3. Verify WAM Trail Backtracking
    // Backtrack to length 1 (unbinds "svdag/relation/1")
    tsfi_anvil_vm_backtrack(&vm, 1);
    printf("  [Anvil Backtrack] Trail Length: %d (Expected 1), Backtrack Triggered: %d (Expected 1)\n", 
           vm.trail_len, vm.backtrack_triggered);
    fflush(stdout);
    assert(vm.trail_len == 1);
    assert(vm.backtrack_triggered == 1);

    // Verify "svdag/relation/1" is unbound (deleted from subgoal table)
    const TSFiSubgoalEntry *unbound_entry = tsfi_anvil_vm_lookup_subgoal(&vm, "svdag/relation/1");
    assert(unbound_entry == NULL);

    // Verify "svdag/relation/0" remains bound
    const TSFiSubgoalEntry *retained_entry = tsfi_anvil_vm_lookup_subgoal(&vm, "svdag/relation/0");
    assert(retained_entry != NULL);

    // 4. Verify Abductive Verification Bytecode Rails
    // P(H)=0.6, P(E)=0.5, Posterior = (0.6*0.8)/0.5 = 0.96.
    // If threshold=0.90, this should pass!
    int bytecode_pass[1] = { 0x5A };
    int status = tsfi_anvil_vm_execute(&vm, bytecode_pass, 1, 0.6f, 0.5f, 0.90f);
    printf("  [Anvil Exec] Pass status: %d (Expected 1)\n", status);
    fflush(stdout);
    assert(status == 1);

    // If threshold=0.98, this should fail and backtrack!
    status = tsfi_anvil_vm_execute(&vm, bytecode_pass, 1, 0.6f, 0.5f, 0.98f);
    printf("  [Anvil Exec] Fail status: %d (Expected 0)\n", status);
    fflush(stdout);
    assert(status == 0);
    assert(vm.trail_len == 0);

    printf("[PASS] WAM Trail backtracking and subgoal memoization verified successfully!\n");
    fflush(stdout);
    return 0;
}
