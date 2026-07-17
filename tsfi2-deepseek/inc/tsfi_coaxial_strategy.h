#ifndef TSFI_COAXIAL_STRATEGY_H
#define TSFI_COAXIAL_STRATEGY_H

#include "tsfi_coaxial_observer.h"
#include "tsfi_priority_queue.h"
#include "tsfi_strategy_lang.h"
#include "tsfi_zmm_vm.h"
#include "tsfi_ramac_layout.h"

// Coaxial Strategy Agent Context
typedef struct {
    TSFiCoaxialObserver *observer;
    TSFiPriorityQueue scheduler_queue;
    TSFiStrategyVM strategy_vm;
    TsfiZmmVmState *zmm;
    int dispatch_count;
    char last_payload[256];
} TSFiCoaxialStrategyAgent;

// Initialize the Coaxial Strategy Agent
void tsfi_coaxial_strategy_agent_init(TSFiCoaxialStrategyAgent *agent, TSFiCoaxialObserver *obs, TsfiZmmVmState *zmm);

// Register incoming observer events into the scheduler queue
int tsfi_coaxial_strategy_agent_collect(TSFiCoaxialStrategyAgent *agent);

// Apply a strategy script to reweight events and dispatch the top event to ZMM
int tsfi_coaxial_strategy_agent_dispatch(TSFiCoaxialStrategyAgent *agent, const char *script);

// Compile decision table strategies for dynamic agent operations
int tsfi_coaxial_strategy_agent_compile_decision_table(TSFiCoaxialStrategyAgent *agent, const tsfi_decision_table *table, uint8_t *bytecode_out, int max_len);

#endif // TSFI_COAXIAL_STRATEGY_H
