#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_coaxial_telemetry.h"

int main(void) {
    printf("[Coaxial Telemetry] Starting monotonic bucket telemetry scheduler tests...\n");
    fflush(stdout);

    // 1. Initialize ZMM and Telemetry Scheduler
    TsfiZmmVmState zmm;
    tsfi_zmm_vm_init(&zmm);

    TSFiCoaxialTelemetryScheduler sched;
    tsfi_coaxial_telemetry_init(&sched, &zmm);

    // 2. Schedule telemetry events with monotonic priorities (2 -> 3)
    int res = tsfi_coaxial_telemetry_schedule(&sched, 2, 32); // Keycode 32, Expire Priority Time 2
    assert(res == 0);
    
    res = tsfi_coaxial_telemetry_schedule(&sched, 3, 30); // Keycode 30, Expire Priority Time 3
    assert(res == 0);

    // 3. Transmit and verify monotonic priority order (Keycode 32 first)
    int status = tsfi_coaxial_telemetry_transmit(&sched);
    printf("  [Telemetry TX 1] Status: %d (Expected 1), Payload: %s\n", status, sched.last_tx_packet);
    fflush(stdout);
    assert(status == 1);
    assert(strstr(sched.last_tx_packet, "KEYCODE=32") != NULL);

    status = tsfi_coaxial_telemetry_transmit(&sched);
    printf("  [Telemetry TX 2] Status: %d (Expected 1), Payload: %s\n", status, sched.last_tx_packet);
    fflush(stdout);
    assert(status == 1);
    assert(strstr(sched.last_tx_packet, "KEYCODE=30") != NULL);

    // Cleanup
    tsfi_zmm_vm_destroy(&zmm);

    printf("[PASS] Monotonic bucket telemetry scheduler verified successfully!\n");
    fflush(stdout);
    return 0;
}
