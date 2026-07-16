#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "tsfi_coaxial_agent.h"

int main(void) {
    printf("[Coaxial Agent] Starting ZMM VM agency and transaction dispatch tests...\n");
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

    TSFiCoaxialAgent agent;
    tsfi_coaxial_agent_init(&agent, &obs, &zmm);

    // 1. Trigger Hardware event
    tsfi_shm_bridge_write(&bridge, 32);

    // 2. Tick the agent to process the event and dispatch to ZMM
    int dispatched = tsfi_coaxial_agent_tick(&agent);
    printf("  [Agent Tick] Dispatched: %d (Expected 1), Payload: %s\n", dispatched, agent.last_dispatched_payload);
    fflush(stdout);
    assert(dispatched == 1);
    assert(agent.transaction_dispatch_count == 1);
    assert(strstr(agent.last_dispatched_payload, "execute_handshake 32") != NULL);

    // Clean up
    tsfi_zmm_vm_destroy(&zmm);

    printf("[PASS] Coaxial agent ZMM dispatch verified successfully!\n");
    fflush(stdout);
    return 0;
}
