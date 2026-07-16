#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "tsfi_anvil_vm.h"
#include "tsfi_strategy_lang.h"

int main(void) {
    printf("[Logic vs Strategy] Starting logic VM and strategy VM integration tests...\n");
    fflush(stdout);

    // 1. Initialize logic VM
    TSFiAnvilVM logic_vm;
    tsfi_anvil_vm_init(&logic_vm);

    // Bind logic coordinates (Logic Phase)
    tsfi_anvil_vm_bind(&logic_vm, "svdag/node/0", "TRUE");
    tsfi_anvil_vm_bind(&logic_vm, "svdag/node/1", "PENDING");
    assert(logic_vm.trail_len == 2);

    // 2. Initialize strategy VM and scheduler queue
    TSFiStrategyVM strategy_vm;
    tsfi_strategy_vm_init(&strategy_vm);

    TSFiPriorityQueue pq;
    tsfi_priority_queue_init(&pq);

    // Push logical subgoals to scheduling queue with original priorities
    tsfi_priority_queue_push(&pq, 10, 0, "node/0");
    tsfi_priority_queue_push(&pq, 2, 1, "node/1");
    assert(pq.size == 2);

    // 3. Execute Strategy script to prioritize abductive search paths
    const char *script = "SET depth 3; SET abductive 10; EVAL;";
    int res = tsfi_strategy_vm_execute(&strategy_vm, &pq, script);
    assert(res == 0);
    assert(strategy_vm.executed_evals == 1);

    // 4. Pop and assert strategy priorities (Item 1: 2*3 + 10 = 16, Item 0: 10*3 + 10 = 40)
    TSFiQueueItem item;
    res = tsfi_priority_queue_pop(&pq, &item);
    assert(res == 0 && item.keycode == 1 && item.priority == 16);
    printf("  [Strategy Assert] Highest priority popped keycode: %d, priority: %d (Expected Keycode: 1, Priority: 16)\n", 
           item.keycode, item.priority);
    fflush(stdout);

    res = tsfi_priority_queue_pop(&pq, &item);
    assert(res == 0 && item.keycode == 0 && item.priority == 40);
    printf("  [Strategy Assert] Next priority popped keycode: %d, priority: %d (Expected Keycode: 0, Priority: 40)\n", 
           item.keycode, item.priority);
    fflush(stdout);

    // 5. Backtrack Logic VM (Logic Phase) -> unbind node/1, node/0 remains bound
    tsfi_anvil_vm_backtrack(&logic_vm, 1);
    assert(logic_vm.trail_len == 1);
    assert(tsfi_anvil_vm_lookup_subgoal(&logic_vm, "svdag/node/1") == NULL);
    assert(tsfi_anvil_vm_lookup_subgoal(&logic_vm, "svdag/node/0") != NULL);
    printf("  [Logic Assert] WAM Backtrack completed successfully (trail len: %d)\n", logic_vm.trail_len);
    fflush(stdout);

    printf("[PASS] Logic and Strategy VMs verified successfully!\n");
    fflush(stdout);
    return 0;
}
