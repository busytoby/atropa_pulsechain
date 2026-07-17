#ifndef TSFI_COAXIAL_OBSERVER_H
#define TSFI_COAXIAL_OBSERVER_H

#include "tsfi_shm_prune.h"
#include "tsfi_anvil_vm.h"
#include "tsfi_ot_accumulator.h"

struct tsfi_fips60_interface;

// Coaxial Event Types
typedef enum {
    COAX_EVENT_HARDWARE, // Hardware register state change
    COAX_EVENT_LOGIC,     // Logic VM binding/backtrack change
    COAX_EVENT_CHANNEL_IO // FIPS 60 I/O channel transition
} TSFiCoaxialEventType;

// Coaxial Event Payload
typedef struct {
    TSFiCoaxialEventType type;
    int payload_value;
    char descriptor[128];
} TSFiCoaxialEvent;

// Coaxial Observer Context
typedef struct {
    TSFiSHMBridge *shm_bridge;
    TSFiAnvilVM *anvil_vm;
    TSFiOTAccumulator *ot_accumulator;
    struct tsfi_fips60_interface *fips_interface;
    int last_observed_keycode;
    int last_observed_trail_len;
    int last_observed_command;
} TSFiCoaxialObserver;

// Initialize Coaxial Observer
void tsfi_coaxial_observer_init(TSFiCoaxialObserver *obs, TSFiSHMBridge *bridge, 
                                TSFiAnvilVM *vm, TSFiOTAccumulator *acc);

// Poll both hardware and logic lines, capturing events coaxially
int tsfi_coaxial_observer_poll(TSFiCoaxialObserver *obs, TSFiCoaxialEvent *event_out);

#endif // TSFI_COAXIAL_OBSERVER_H
