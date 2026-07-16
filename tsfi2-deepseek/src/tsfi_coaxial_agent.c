#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_coaxial_agent.h"

void tsfi_coaxial_agent_init(TSFiCoaxialAgent *agent, TSFiCoaxialObserver *obs, TsfiZmmVmState *zmm) {
    if (!agent) return;
    agent->observer = obs;
    agent->zmm = zmm;
    agent->transaction_dispatch_count = 0;
    memset(agent->last_dispatched_payload, 0, sizeof(agent->last_dispatched_payload));
}

int tsfi_coaxial_agent_tick(TSFiCoaxialAgent *agent) {
    if (!agent || !agent->observer || !agent->zmm) return -1;

    TSFiCoaxialEvent event;
    int event_status = tsfi_coaxial_observer_poll(agent->observer, &event);

    if (event_status == 1) {
        // Observer captured an event -> Agent resolves dynamic agency dispatch
        char zmm_tx_payload[256];
        
        if (event.type == COAX_EVENT_HARDWARE) {
            // Hardware event (SCSI keycode writes) trigger Winchester contract calls on ZMM
            snprintf(zmm_tx_payload, sizeof(zmm_tx_payload), 
                     "CALL contract_winchester_mq execute_handshake %d", event.payload_value);
        } else if (event.type == COAX_EVENT_LOGIC) {
            // Logic VM trail changes trigger abductive state mutations on ZMM
            snprintf(zmm_tx_payload, sizeof(zmm_tx_payload), 
                     "MUTATE state_accumulator set_trail_len %d", event.payload_value);
        } else {
            return 0; // Unhandled event
        }

        // Dispatch transaction to general ZMM VM space (mock state mutation execution)
        snprintf(agent->last_dispatched_payload, sizeof(agent->last_dispatched_payload), "%s", zmm_tx_payload);
        agent->transaction_dispatch_count++;

        // Log transaction outcome directly to OT Accumulator
        if (agent->observer->ot_accumulator) {
            char coord[128];
            snprintf(coord, sizeof(coord), "svdag/zmm_tx/%d", agent->transaction_dispatch_count);
            tsfi_ot_accumulator_add(agent->observer->ot_accumulator, coord, 0.97f);
        }

        return 1; // Dispatched transaction successfully
    }

    return 0; // No events to process
}
