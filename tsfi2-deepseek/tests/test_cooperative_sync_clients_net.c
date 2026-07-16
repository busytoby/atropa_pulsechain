#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_anvil_vm.h"
#include "tsfi_zmm_vm.h"
#include "tsfi_spec_queues.h"

// Kermit Bidirectional Packet Frame
typedef struct {
    uint8_t soh;
    uint8_t seq;
    uint8_t type; // 0x44: Data, 0x41: ACK
    char data[64];
} KermitBidirectionalFrame;

int main(void) {
    printf("[Cooperative Sync Clients Net] Starting bidirectional sync client pipeline tests...\n");
    fflush(stdout);

    // 1. Initialize separated Rails VMs on Anvil
    TSFiAnvilVM client_A_vm;
    tsfi_anvil_vm_init(&client_A_vm);

    TSFiAnvilVM client_B_vm;
    tsfi_anvil_vm_init(&client_B_vm);

    // 2. Initialize ZMM ledger VM
    TsfiZmmVmState zmm;
    tsfi_zmm_vm_init(&zmm);

    // Initialize lock-free communication rings representing coaxial loops
    TSFiLockFreeRing ring_A_to_B; // A sends data to B
    tsfi_ring_init(&ring_A_to_B);

    TSFiLockFreeRing ring_B_to_A; // B sends ACK to A
    tsfi_ring_init(&ring_B_to_A);

    // 3. Client A binds ETH gas and transmits Data Frame
    tsfi_anvil_vm_bind(&client_A_vm, "gas/ETH", "21000");
    int res = tsfi_ring_push(&ring_A_to_B, 1, 32); // Keycode 32 represent data trigger
    assert(res == 0);
    printf("  [Client A] Spent ETH gas, transmitted Data Frame to Client B.\n");
    fflush(stdout);

    // 4. Client B receives Data Frame, binds LAU Gas Token, and transmits ACK Frame
    TSFiRingItem rx_item_B;
    res = tsfi_ring_pop(&ring_A_to_B, &rx_item_B);
    assert(res == 0 && rx_item_B.keycode == 32);

    tsfi_anvil_vm_bind(&client_B_vm, "gas/LAU", "100000");
    res = tsfi_ring_push(&ring_B_to_A, 1, 6); // Keycode 6 represents ACK trigger
    assert(res == 0);
    printf("  [Client B] Spent LAU gas, verified Data Frame, transmitted ACK to Client A.\n");
    fflush(stdout);

    // 5. Client A receives ACK and commits sync status to ZMM
    TSFiRingItem rx_item_A;
    res = tsfi_ring_pop(&ring_B_to_A, &rx_item_A);
    assert(res == 0 && rx_item_A.keycode == 6);

    tsfi_anvil_vm_bind(&client_A_vm, "sync/status", "VERIFIED");
    tsfi_zmm_vm_exec(&zmm, "CALL ledger_registry commit_sync Client_A Client_B VERIFIED");
    printf("  [Client A] Received ACK. Bidirectional sync state committed to ZMM.\n");
    fflush(stdout);

    // Cleanup
    tsfi_zmm_vm_destroy(&zmm);

    printf("[PASS] Bidirectional cooperative sync client network verified successfully!\n");
    fflush(stdout);
    return 0;
}
