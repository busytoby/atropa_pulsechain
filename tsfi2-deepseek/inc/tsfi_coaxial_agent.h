#ifndef TSFI_COAXIAL_AGENT_H
#define TSFI_COAXIAL_AGENT_H

#include "tsfi_coaxial_observer.h"
#include "tsfi_zmm_vm.h"

// Coaxial Agent Context
typedef struct {
    TSFiCoaxialObserver *observer;
    TsfiZmmVmState *zmm;
    int transaction_dispatch_count;
    char last_dispatched_payload[256];
} TSFiCoaxialAgent;

// Initialize the Coaxial Agent Context
void tsfi_coaxial_agent_init(TSFiCoaxialAgent *agent, TSFiCoaxialObserver *obs, TsfiZmmVmState *zmm);

// Step the agent: Polls the observer, decides, and dispatches transactions to the general ZMM
int tsfi_coaxial_agent_tick(TSFiCoaxialAgent *agent);

#endif // TSFI_COAXIAL_AGENT_H
