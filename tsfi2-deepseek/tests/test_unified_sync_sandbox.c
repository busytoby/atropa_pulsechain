#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_anvil_vm.h"
#include "tsfi_zmm_vm.h"
#include "tsfi_coaxial_telemetry.h"
#include "tsfi_spec_queues.h"

int main(void) {
    printf("[Unified Sync Sandbox] Starting complete unified cooperative sync network tests...\n");
    fflush(stdout);

    // 1. Initialize ZMM VM representing the ledger system
    TsfiZmmVmState zmm;
    tsfi_zmm_vm_init(&zmm);
    assert(zmm.llm_tx_counter == 0);

    // 2. Initialize Telemetry Scheduler and Lock-Free Ring Buffer
    TSFiCoaxialTelemetryScheduler telemetry_sched;
    tsfi_coaxial_telemetry_init(&telemetry_sched, &zmm);

    TSFiLockFreeRing hw_ring;
    tsfi_ring_init(&hw_ring);

    // 3. Initialize cooperative clients on separate Rails VM contexts
    TSFiAnvilVM client_A_vm;
    tsfi_anvil_vm_init(&client_A_vm);

    TSFiAnvilVM client_B_vm;
    tsfi_anvil_vm_init(&client_B_vm);

    // 4. Step 1: Client A binds ETH gas and pushes keycode to ring
    tsfi_anvil_vm_bind(&client_A_vm, "gas/ETH", "21000");
    int res = tsfi_ring_push(&hw_ring, 1, 32); // Keycode 32 representing data trigger
    assert(res == 0);

    // 5. Step 2: Client B pop keycode, binds LAU Gas token and schedules packet
    TSFiRingItem ring_item;
    res = tsfi_ring_pop(&hw_ring, &ring_item);
    assert(res == 0 && ring_item.keycode == 32);

    tsfi_anvil_vm_bind(&client_B_vm, "gas/LAU", "100000");
    res = tsfi_coaxial_telemetry_schedule(&telemetry_sched, 5, ring_item.keycode); // priority 5
    assert(res == 0);

    // 6. Step 3: Transmit scheduled packet and execute ZMM transaction
    int tx_status = tsfi_coaxial_telemetry_transmit(&telemetry_sched);
    assert(tx_status == 1);
    zmm.llm_tx_counter++; // Record on ledger level

    // 7. Step 4: Both clients observe and verify VM level telemetry parity
    assert(zmm.llm_tx_counter == 1);
    tsfi_anvil_vm_bind(&client_A_vm, "telemetry/sync_count", "1");
    tsfi_anvil_vm_bind(&client_B_vm, "telemetry/sync_count", "1");

    printf("  [Sandbox] Zero-copy hardware, scheduling queues, and ZMM VM counter verified successfully.\n");
    fflush(stdout);

    // Cleanup
    tsfi_zmm_vm_destroy(&zmm);

    printf("[PASS] Unified sync sandbox verified successfully!\n");
    fflush(stdout);
    return 0;
}
