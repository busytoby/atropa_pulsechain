#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_anvil_vm.h"
#include "tsfi_strategy_lang.h"

int main(void) {
    printf("[Rails VM Strategy] Deploying strategy functions on the Rails VM...\n");
    fflush(stdout);

    // 1. Initialize Anvil VM representing the Black Rail (Constraint Logic)
    TSFiAnvilVM black_rail_vm;
    tsfi_anvil_vm_init(&black_rail_vm);

    // 2. Initialize Strategy VM representing the Red Rail (Abductive Strategy)
    TSFiStrategyVM red_rail_strategy;
    tsfi_strategy_vm_init(&red_rail_strategy);

    TSFiPriorityQueue scheduler_queue;
    tsfi_priority_queue_init(&scheduler_queue);

    // Bind coordinate targets to the Black Rail WAM Trail
    tsfi_anvil_vm_bind(&black_rail_vm, "svdag/black_rail/coord_0", "TRUE");
    tsfi_anvil_vm_bind(&black_rail_vm, "svdag/black_rail/coord_1", "PENDING");
    assert(black_rail_vm.trail_len == 2);

    // Push candidate coordinates to Red Rail Scheduler Queue
    tsfi_priority_queue_push(&scheduler_queue, 15, 500, "coord_0");
    tsfi_priority_queue_push(&scheduler_queue, 5, 600, "coord_1");
    assert(scheduler_queue.size == 2);

    // 3. Black Rail Constraint Check: Evaluate abductive limits
    int bytecode[1] = { 0x5A }; // OP_VERIFY_ABDUCTIVE
    // Posterior = (0.5 * 0.8) / 0.6 = 0.66 < 0.95 -> Triggers rollback!
    int status = tsfi_anvil_vm_execute(&black_rail_vm, bytecode, 1, 0.5f, 0.6f, 0.95f);
    assert(status == 0);
    assert(black_rail_vm.trail_len == 0);
    printf("  [Black Rail] Constraints failed. WAM Trail rolled back to 0 successfully.\n");
    fflush(stdout);

    // 4. Red Rail Strategy Modification: Boost abductive path scheduling priority
    const char *script = "SET abductive 30; EVAL;";
    int res = tsfi_strategy_vm_execute(&red_rail_strategy, &scheduler_queue, script);
    assert(res == 0);

    // Verify Red Rail priorities updated:
    // coord_1 (600): (5 * 1) + 30 = 35
    // coord_0 (500): (15 * 1) + 30 = 45
    TSFiQueueItem item;
    res = tsfi_priority_queue_pop(&scheduler_queue, &item);
    assert(res == 0 && item.keycode == 600 && item.priority == 35);
    printf("  [Red Rail] Highest strategy path popped: keycode %d, priority: %d (Expected Keycode: 600, Priority: 35)\n",
           item.keycode, item.priority);
    fflush(stdout);

    printf("[PASS] Rails VM strategy functions executed successfully!\n");
    fflush(stdout);
    return 0;
}
