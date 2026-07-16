#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_coaxial_strategy.h"

int main(void) {
    printf("[Coaxial Strategy] Starting coaxial strategy agent dispatch tests...\n");
    fflush(stdout);

    // Context Setup
    TSFiSHMBridge bridge;
    tsfi_shm_bridge_init(&bridge);

    TSFiAnvilVM vm;
    tsfi_anvil_vm_init(&vm);

    TSFiOTAccumulator acc;
    tsfi_ot_accumulator_init(&acc);

    TSFiCoaxialObserver obs;
    tsfi_coaxial_observer_init(&obs, &bridge, &vm, &acc);

    TsfiZmmVmState zmm;
    tsfi_zmm_vm_init(&zmm);

    TSFiCoaxialStrategyAgent agent;
    tsfi_coaxial_strategy_agent_init(&agent, &obs, &zmm);

    // 1. Trigger & Collect Logic Event (Default priority 15)
    tsfi_anvil_vm_bind(&vm, "svdag/relation/0", "TRUE");
    int collected = tsfi_coaxial_strategy_agent_collect(&agent);
    assert(collected == 1);

    // 2. Trigger & Collect Hardware Event (Default priority 5)
    tsfi_shm_bridge_write(&bridge, 30);
    collected = tsfi_coaxial_strategy_agent_collect(&agent);
    assert(collected == 1);
    assert(agent.scheduler_queue.size == 2);

    // 3. Dispatch using Strategy script: "SET abductive 10; EVAL;"
    // path_B (Hardware): 5 * 1 + 10 = 15
    // path_A (Logic): 15 * 1 + 10 = 25
    // Hardware keycode 30 (priority 15) should be dispatched first!
    int status = tsfi_coaxial_strategy_agent_dispatch(&agent, "SET abductive 10; EVAL;");
    printf("  [Strategy Dispatch 1] Status: %d (Expected 1), Payload: %s\n", status, agent.last_payload);
    fflush(stdout);
    assert(status == 1);
    assert(strstr(agent.last_payload, "30 15") != NULL);

    // Clean up
    tsfi_zmm_vm_destroy(&zmm);

    printf("[PASS] Coaxial strategy agent dispatch verified successfully!\n");
    fflush(stdout);
    return 0;
}
