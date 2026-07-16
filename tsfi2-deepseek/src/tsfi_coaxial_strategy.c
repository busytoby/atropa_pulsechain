#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_coaxial_strategy.h"

void tsfi_coaxial_strategy_agent_init(TSFiCoaxialStrategyAgent *agent, TSFiCoaxialObserver *obs, TsfiZmmVmState *zmm) {
    if (!agent) return;
    agent->observer = obs;
    agent->zmm = zmm;
    agent->dispatch_count = 0;
    tsfi_priority_queue_init(&agent->scheduler_queue);
    tsfi_strategy_vm_init(&agent->strategy_vm);
    memset(agent->last_payload, 0, sizeof(agent->last_payload));
}

int tsfi_coaxial_strategy_agent_collect(TSFiCoaxialStrategyAgent *agent) {
    if (!agent || !agent->observer) return -1;

    TSFiCoaxialEvent event;
    int status = tsfi_coaxial_observer_poll(agent->observer, &event);
    if (status == 1) {
        // Base priority values: Hardware = 5 (higher priority), Logic = 15 (lower priority)
        int priority = (event.type == COAX_EVENT_HARDWARE) ? 5 : 15;
        tsfi_priority_queue_push(&agent->scheduler_queue, priority, event.payload_value, event.descriptor);
        return 1;
    }
    return 0; // No events to collect
}

int tsfi_coaxial_strategy_agent_dispatch(TSFiCoaxialStrategyAgent *agent, const char *script) {
    if (!agent || !agent->zmm || agent->scheduler_queue.size <= 0) return -1;

    // Apply strategy script reweighting if provided
    if (script) {
        tsfi_strategy_vm_execute(&agent->strategy_vm, &agent->scheduler_queue, script);
    }

    // Pop the highest priority item from scheduler
    TSFiQueueItem item;
    int res = tsfi_priority_queue_pop(&agent->scheduler_queue, &item);
    if (res == 0) {
        char payload[256];
        snprintf(payload, sizeof(payload), "CALL contract_strategy execute_prioritized %d %d", item.keycode, item.priority);
        
        snprintf(agent->last_payload, sizeof(agent->last_payload), "%s", payload);
        agent->dispatch_count++;
        return 1;
    }

    return 0;
}
