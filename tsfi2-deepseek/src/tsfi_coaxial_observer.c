#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_coaxial_observer.h"

void tsfi_coaxial_observer_init(TSFiCoaxialObserver *obs, TSFiSHMBridge *bridge, 
                                TSFiAnvilVM *vm, TSFiOTAccumulator *acc) {
    if (!obs) return;
    obs->shm_bridge = bridge;
    obs->anvil_vm = vm;
    obs->ot_accumulator = acc;
    obs->last_observed_keycode = -1;
    obs->last_observed_trail_len = 0;
}

int tsfi_coaxial_observer_poll(TSFiCoaxialObserver *obs, TSFiCoaxialEvent *event_out) {
    if (!obs || !event_out) return -1;

    // 1. Observe Hardware Line (WinchesterMQ SHM bridge)
    if (obs->shm_bridge) {
        int current_keycode = obs->shm_bridge->keycode_register;
        if (current_keycode != obs->last_observed_keycode && obs->shm_bridge->status_register == 1) {
            obs->last_observed_keycode = current_keycode;
            event_out->type = COAX_EVENT_HARDWARE;
            event_out->payload_value = current_keycode;
            snprintf(event_out->descriptor, 127, "Hardware register keycode write: %d", current_keycode);

            // Accumulate to OT Accumulator path log
            if (obs->ot_accumulator) {
                char coord[128];
                snprintf(coord, sizeof(coord), "svdag/hw_reg/%d", current_keycode);
                tsfi_ot_accumulator_add(obs->ot_accumulator, coord, 0.99f);
            }
            return 1; // Event captured
        }
    }

    // 2. Observe Logic Line (Anvil VM Trail)
    if (obs->anvil_vm) {
        int current_trail = obs->anvil_vm->trail_len;
        if (current_trail != obs->last_observed_trail_len) {
            obs->last_observed_trail_len = current_trail;
            event_out->type = COAX_EVENT_LOGIC;
            event_out->payload_value = current_trail;
            snprintf(event_out->descriptor, 127, "Logic VM variable trail length: %d", current_trail);

            // Accumulate to OT Accumulator path log
            if (obs->ot_accumulator) {
                char coord[128];
                snprintf(coord, sizeof(coord), "svdag/vm_trail/%d", current_trail);
                tsfi_ot_accumulator_add(obs->ot_accumulator, coord, 0.95f);
            }
            return 1; // Event captured
        }
    }

    return 0; // No new events observed
}
