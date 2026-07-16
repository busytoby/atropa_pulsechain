#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "tsfi_anvil_vm.h"
#include "tsfi_strategy_lang.h"

int main(void) {
    printf("[Strategy Contract] Deploying abductive strategy contract to Anvil...\n");
    fflush(stdout);

    // 1. Instantiate VM States representing the deployed contract
    TSFiAnvilVM logic_vm;
    tsfi_anvil_vm_init(&logic_vm);

    TSFiStrategyVM strategy_vm;
    tsfi_strategy_vm_init(&strategy_vm);

    TSFiPriorityQueue pq;
    tsfi_priority_queue_init(&pq);

    // 2. Bind candidate paths to WAM logic registers
    tsfi_anvil_vm_bind(&logic_vm, "svdag/contract_path/A", "PENDING");
    tsfi_anvil_vm_bind(&logic_vm, "svdag/contract_path/B", "PENDING");
    assert(logic_vm.trail_len == 2);

    // Push coordinates to strategy scheduling queue
    tsfi_priority_queue_push(&pq, 8, 100, "path_A"); // Initial priority 8
    tsfi_priority_queue_push(&pq, 3, 200, "path_B"); // Initial priority 3 (higher priority)
    assert(pq.size == 2);

    // 3. Execute logic abductive verification rails (Asserting threshold = 0.90)
    // If we run validation and it fails, it should unbind all variables on Anvil
    int bytecode[1] = { 0x5A }; // OP_VERIFY_ABDUCTIVE
    int status = tsfi_anvil_vm_execute(&logic_vm, bytecode, 1, 0.4f, 0.5f, 0.90f); 
    // Posterior = (0.4*0.8)/0.5 = 0.64 < 0.90 -> Should fail and backtrack!
    assert(status == 0);
    assert(logic_vm.trail_len == 0);
    printf("  [Contract Logic] Abductive constraints failed as expected. WAM Trail rolled back to 0.\n");
    fflush(stdout);

    // 4. Run Strategy Modulation Script (Prioritizing abductive bias)
    const char *strat_script = "SET abductive 20; EVAL;";
    int res = tsfi_strategy_vm_execute(&strategy_vm, &pq, strat_script);
    assert(res == 0);

    // Verify priorities are correctly updated:
    // path_B (200): (3 * 1) + 20 = 23
    // path_A (100): (8 * 1) + 20 = 28
    TSFiQueueItem popped_item;
    res = tsfi_priority_queue_pop(&pq, &popped_item);
    assert(res == 0 && popped_item.keycode == 200 && popped_item.priority == 23);
    printf("  [Contract Strategy] Popped highest strategy path keycode: %d, priority: %d (Expected Keycode: 200, Priority: 23)\n",
           popped_item.keycode, popped_item.priority);
    fflush(stdout);

    printf("[PASS] Abductive strategy contract verified successfully on Anvil!\n");
    fflush(stdout);
    return 0;
}
