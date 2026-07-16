#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_strategy_lang.h"

int main(void) {
    printf("[Strategy Lang] Starting strategy script parser tests...\n");
    fflush(stdout);

    TSFiStrategyVM vm;
    tsfi_strategy_vm_init(&vm);

    TSFiPriorityQueue pq;
    tsfi_priority_queue_init(&pq);

    tsfi_priority_queue_push(&pq, 10, 30, "LOW_PRIO");
    tsfi_priority_queue_push(&pq, 2, 31, "HIGH_PRIO");

    // Execute script: set depth scale to 2, abductive scale to 5, and execute EVAL re-prioritization
    const char *script = "SET depth 2; SET abductive 5; EVAL;";
    int res = tsfi_strategy_vm_execute(&vm, &pq, script);
    assert(res == 0);
    printf("  [Strategy Script] Executed eval: %d (Expected 1), Depth Scale: %d (Expected 2)\n", 
           vm.executed_evals, vm.depth_priority_scale);
    fflush(stdout);
    assert(vm.executed_evals == 1);
    assert(vm.depth_priority_scale == 2);
    assert(vm.abductive_priority_scale == 5);

    // Pop and verify new priorities:
    // Item 1: (2 * 2) + 5 = 9
    // Item 2: (10 * 2) + 5 = 25
    TSFiQueueItem item;
    res = tsfi_priority_queue_pop(&pq, &item);
    assert(res == 0);
    printf("  [PQ Pop 1] Keycode: %d, Priority: %d (Expected Priority: 9)\n", item.keycode, item.priority);
    fflush(stdout);
    assert(item.priority == 9);

    res = tsfi_priority_queue_pop(&pq, &item);
    assert(res == 0);
    printf("  [PQ Pop 2] Keycode: %d, Priority: %d (Expected Priority: 25)\n", item.keycode, item.priority);
    fflush(stdout);
    assert(item.priority == 25);

    printf("[PASS] Strategy script execution verified successfully!\n");
    fflush(stdout);
    return 0;
}
