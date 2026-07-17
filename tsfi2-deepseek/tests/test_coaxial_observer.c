#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "tsfi_coaxial_observer.h"
#include "tsfi_mainframe_v370.h"

int main(void) {
    printf("[Coaxial Observer] Starting hardware and logic VM dual-line observation tests...\n");
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

    TSFiCoaxialEvent event;

    // 1. Trigger Hardware Event (Write keycode 30)
    tsfi_shm_bridge_write(&bridge, 30);
    int captured = tsfi_coaxial_observer_poll(&obs, &event);
    printf("  [Coaxial Watch] Hardware captured: %d, Descriptor: %s\n", captured, event.descriptor);
    fflush(stdout);
    assert(captured == 1);
    assert(event.type == COAX_EVENT_HARDWARE);
    assert(event.payload_value == 30);

    // 2. Trigger Logic Event (Bind coordinate value)
    tsfi_anvil_vm_bind(&vm, "svdag/relation/0", "TRUE");
    captured = tsfi_coaxial_observer_poll(&obs, &event);
    printf("  [Coaxial Watch] Logic captured: %d, Descriptor: %s\n", captured, event.descriptor);
    fflush(stdout);
    assert(captured == 1);
    assert(event.type == COAX_EVENT_LOGIC);
    assert(event.payload_value == 1);

    // 3. Trigger FIPS 60 Channel I/O Event
    tsfi_fips60_interface fips;
    tsfi_fips60_init(&fips);
    obs.fips_interface = &fips;
    
    assert(tsfi_fips60_bus_out(&fips, 0x01, NULL, 0) == 0); // WRITE command
    captured = tsfi_coaxial_observer_poll(&obs, &event);
    printf("  [Coaxial Watch] FIPS 60 captured: %d, Descriptor: %s\n", captured, event.descriptor);
    fflush(stdout);
    assert(captured == 1);
    assert(event.type == COAX_EVENT_CHANNEL_IO);
    assert(event.payload_value == 0x01);

    // 4. Verify accumulator contains observations from all lines
    printf("  [Coaxial Watch] OT Accumulator potential: %.2f (Expected 2.91)\n", tsfi_ot_accumulator_get_potential(&acc));
    fflush(stdout);
    assert(acc.count == 3);
    // Potential = 0.99 (Hardware) + 0.95 (Logic) + 0.97 (Channel I/O) = 2.91
    assert(fabs(tsfi_ot_accumulator_get_potential(&acc) - 2.91f) < 0.0001f);

    printf("[PASS] Coaxial observer verified successfully!\n");
    fflush(stdout);
    return 0;
}
