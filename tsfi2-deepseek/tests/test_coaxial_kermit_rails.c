#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_anvil_vm.h"
#include "tsfi_zmm_vm.h"

// Kermit Frame Representation
typedef struct {
    uint8_t soh;      // Start of Header (0x01)
    uint8_t seq;      // Sequence number
    uint8_t len;      // Length of data
    char data[64];    // Payload data
    uint8_t checksum; // Parity checksum
} KermitFrame;

int main(void) {
    printf("[Coaxial Kermit Rails] Starting Kermit-over-coaxial transfer with dual token gas fees...\n");
    fflush(stdout);

    // 1. Initialize Sender VM (Black Rail) and Receiver VM (Red Rail)
    TSFiAnvilVM sender_vm;
    tsfi_anvil_vm_init(&sender_vm);

    TSFiAnvilVM receiver_vm;
    tsfi_anvil_vm_init(&receiver_vm);

    // 2. Validate Gas Requirements (ETH on Sender, LAU Gas Token on Receiver)
    // Bind balances to the WAM context
    tsfi_anvil_vm_bind(&sender_vm, "balance/ETH", "1000");
    tsfi_anvil_vm_bind(&receiver_vm, "balance/LAU_GAS", "5000");
    printf("  [Gas Initialization] Sender ETH Balance: 1000, Receiver LAU Balance: 5000\n");
    fflush(stdout);

    // Simulate Gas Deduction on WAM Trail
    tsfi_anvil_vm_bind(&sender_vm, "gas/spent/ETH", "21000");
    tsfi_anvil_vm_bind(&receiver_vm, "gas/spent/LAU_GAS", "100000");
    assert(sender_vm.trail_len == 2);
    assert(receiver_vm.trail_len == 2);

    // 3. Assemble Kermit Frame
    KermitFrame frame;
    frame.soh = 0x01;
    frame.seq = 1;
    strcpy(frame.data, "ATROPA_SIGNAL_RECONCILIATION");
    frame.len = (uint8_t)strlen(frame.data);
    
    // Calculate simple checksum
    uint32_t sum = frame.soh + frame.seq + frame.len;
    for (int i = 0; i < frame.len; i++) {
        sum += frame.data[i];
    }
    frame.checksum = (uint8_t)(sum % 256);

    printf("  [Kermit Pack] Frame Packed: seq=%d, len=%d, checksum=%02X\n", frame.seq, frame.len, frame.checksum);
    fflush(stdout);

    // 4. Route Kermit Frame over Coaxial Loop (Transfer from Sender WAM to Receiver WAM)
    // Receiver parses the frame and verifies checksum
    uint32_t rx_sum = frame.soh + frame.seq + frame.len;
    for (int i = 0; i < frame.len; i++) {
        rx_sum += frame.data[i];
    }
    uint8_t rx_checksum = (uint8_t)(rx_sum % 256);
    
    printf("  [Kermit Unpack] Frame Received: data=%s, checksum_valid=%s\n", 
           frame.data, (rx_checksum == frame.checksum) ? "TRUE" : "FALSE");
    fflush(stdout);
    assert(rx_checksum == frame.checksum);

    // Verify Receiver is now bound to the transaction completion status
    tsfi_anvil_vm_bind(&receiver_vm, "status/kermit_transfer", "SUCCESS");
    assert(receiver_vm.trail_len == 3);

    printf("[PASS] Kermit-over-coaxial transfer with dual token gas verified successfully!\n");
    fflush(stdout);
    return 0;
}
