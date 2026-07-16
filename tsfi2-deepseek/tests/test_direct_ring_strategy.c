#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_winchester_bridge.h"
#include "tsfi_coaxial_strategy.h"

int main(void) {
    printf("[Direct Ring Pipeline] Starting zero-copy ring strategy pipeline tests...\n");
    fflush(stdout);

    // 1. Initialize Engine and WinchesterMQ Bridge
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiSynthPerfEngine *perf_engine = tsfi_synth_perf_create(dat, trie_root);
    assert(perf_engine != NULL);

    TSFiWinchesterBridge *bridge = tsfi_winchester_bridge_create(perf_engine);
    assert(bridge != NULL);

    // 2. Initialize Strategy Agent and ZMM
    TsfiZmmVmState zmm;
    tsfi_zmm_vm_init(&zmm);

    TSFiCoaxialObserver obs;
    TSFiSHMBridge shm_mock;
    tsfi_shm_bridge_init(&shm_mock);

    TSFiAnvilVM vm;
    tsfi_anvil_vm_init(&vm);

    TSFiOTAccumulator acc;
    tsfi_ot_accumulator_init(&acc);

    tsfi_coaxial_observer_init(&obs, &shm_mock, &vm, &acc);

    TSFiCoaxialStrategyAgent agent;
    tsfi_coaxial_strategy_agent_init(&agent, &obs, &zmm);

    // 3. Write keycode event to the WinchesterMQ Lock-Free Ring directly
    int res = tsfi_ring_push(&bridge->event_ring, 5, 32); // Keycode 32 (D), Priority 5
    assert(res == 0);

    // 4. Zero-copy read from the bridge's event ring into the Strategy Agent scheduler queue
    TSFiRingItem ring_item;
    res = tsfi_ring_pop(&bridge->event_ring, &ring_item);
    assert(res == 0);
    assert(ring_item.keycode == 32);

    // Push popped keycode directly to agent's queue
    tsfi_priority_queue_push(&agent.scheduler_queue, ring_item.priority, ring_item.keycode, "direct_ring_event");
    assert(agent.scheduler_queue.size == 1);

    // 5. Dispatch top-priority action to ZMM
    int status = tsfi_coaxial_strategy_agent_dispatch(&agent, "SET abductive 10; EVAL;");
    printf("  [Direct Pipeline Dispatch] Status: %d, Payload: %s\n", status, agent.last_payload);
    fflush(stdout);
    assert(status == 1);
    assert(strstr(agent.last_payload, "32 15") != NULL); // keycode 32, modulated priority 15 (5 + 10)

    // Cleanup
    tsfi_winchester_bridge_destroy(bridge);
    tsfi_synth_perf_destroy(perf_engine);
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);
    tsfi_zmm_vm_destroy(&zmm);

    printf("[PASS] Zero-copy ring strategy pipeline verified successfully!\n");
    fflush(stdout);
    return 0;
}
