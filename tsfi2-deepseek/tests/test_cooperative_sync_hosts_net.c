#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_anvil_vm.h"
#include "tsfi_zmm_vm.h"
#include "tsfi_coaxial_telemetry.h"
#include "tsfi_spec_queues.h"

int main(void) {
    printf("[Cooperative Sync Hosts Net] Starting comprehensive sync hosts network pipeline tests...\n");
    fflush(stdout);

    // 1. Setup ZMM VM context
    TsfiZmmVmState zmm;
    tsfi_zmm_vm_init(&zmm);

    // 2. Setup Telemetry Scheduler and Lock-Free Ring Queue
    TSFiCoaxialTelemetryScheduler telemetry_sched;
    tsfi_coaxial_telemetry_init(&telemetry_sched, &zmm);

    TSFiLockFreeRing hw_ring;
    tsfi_ring_init(&hw_ring);

    // 3. Host A (Initiator) pushes keycode event onto lock-free hardware ring
    int res = tsfi_ring_push(&hw_ring, 1, 30); // Keycode 30
    assert(res == 0);

    // 4. Host B (Verifier) extracts event from ring and schedules it
    TSFiRingItem ring_item;
    res = tsfi_ring_pop(&hw_ring, &ring_item);
    assert(res == 0);
    assert(ring_item.keycode == 30);

    // Schedule verification event in monotonic scheduler (time priority 5)
    res = tsfi_coaxial_telemetry_schedule(&telemetry_sched, 5, ring_item.keycode);
    assert(res == 0);

    // 5. Transmit scheduled packet and assert ZMM logging
    int tx_status = tsfi_coaxial_telemetry_transmit(&telemetry_sched);
    printf("  [Cooperative TX] Status: %d (Expected 1), Payload: %s\n", tx_status, telemetry_sched.last_tx_packet);
    fflush(stdout);
    assert(tx_status == 1);
    assert(strstr(telemetry_sched.last_tx_packet, "KEYCODE=30") != NULL);

    // Cleanup
    tsfi_zmm_vm_destroy(&zmm);

    printf("[PASS] Comprehensive sync hosts network pipeline verified successfully!\n");
    fflush(stdout);
    return 0;
}
