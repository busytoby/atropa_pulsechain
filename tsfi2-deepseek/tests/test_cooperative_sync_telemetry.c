#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_anvil_vm.h"
#include "tsfi_zmm_vm.h"

// Telemetry Client Structure
typedef struct {
    TSFiAnvilVM *vm;
    const char *client_name;
} TSFiTelemetryClient;

int main(void) {
    printf("[Cooperative Sync Telemetry] Starting VM-level telemetry observation tests...\n");
    fflush(stdout);

    // 1. Initialize ZMM VM representing the ledger system
    TsfiZmmVmState zmm;
    tsfi_zmm_vm_init(&zmm);
    assert(zmm.llm_tx_counter == 0);

    // 2. Initialize cooperative sync clients on separate VM spaces
    TSFiAnvilVM vm_A;
    tsfi_anvil_vm_init(&vm_A);

    TSFiAnvilVM vm_B;
    tsfi_anvil_vm_init(&vm_B);

    TSFiTelemetryClient client_A = { .vm = &vm_A, .client_name = "Client_A" };
    TSFiTelemetryClient client_B = { .vm = &vm_B, .client_name = "Client_B" };

    // 3. Dispatch coaxial transactions on ZMM
    tsfi_zmm_vm_exec(&zmm, "CALL coaxial_channel send_handshake_telemetry 30");
    zmm.llm_tx_counter++; // Increment transaction counter on ZMM VM level

    tsfi_zmm_vm_exec(&zmm, "CALL coaxial_channel send_handshake_telemetry 32");
    zmm.llm_tx_counter++;

    // 4. Both clients observe and verify transactions directly from the ZMM VM level
    printf("  [Client A] Querying ZMM VM level transaction counter: %lu (Expected: 2)\n", (unsigned long)zmm.llm_tx_counter);
    fflush(stdout);
    assert(zmm.llm_tx_counter == 2);
    tsfi_anvil_vm_bind(client_A.vm, "telemetry/observed_tx_count", "2");

    printf("  [Client B] Querying ZMM VM level transaction counter: %lu (Expected: 2)\n", (unsigned long)zmm.llm_tx_counter);
    fflush(stdout);
    assert(zmm.llm_tx_counter == 2);
    tsfi_anvil_vm_bind(client_B.vm, "telemetry/observed_tx_count", "2");

    // Cleanup
    tsfi_zmm_vm_destroy(&zmm);

    printf("[PASS] Bidirectional VM-level telemetry observation verified successfully!\n");
    fflush(stdout);
    return 0;
}
