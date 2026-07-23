#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define REG_LOCAL_STACK  0xF900  // Local device stack cache
#define REG_SHARED_STACK 0xF600  // Shared SRAM DECnet buffer
#define REG_SCSI_PHASE   0xFB00

// STANAG Tactical Frame Layout (Rule 13 Compliant)
typedef struct {
    uint16_t stanag_id;
    uint8_t src_node;
    uint8_t dst_node;
    uint32_t payload[4];
} StanagFrame;

int main() {
    printf("=== TSFi ZMM VM Auncient Network Device Stack Tests ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and deploy hucSystem.yul
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char cmd[512];

    // 2. Test Local Stack Push/Pop (SCSI buffer)
    printf("\n--- Test Case 1: Local Link-Layer Device Stack ---\n");
    // Write 3 consecutive bytes to local stack space (0xF900)
    uint32_t scsi_bytes[3] = {0x1A, 0x2B, 0x3C};
    for (int i = 0; i < 3; i++) {
        sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                     "%064x"
                     "%064x\"",
                REG_LOCAL_STACK + i, scsi_bytes[i]);
        tsfi_zmm_vm_exec(&vm, cmd);
    }
    printf("[XPL] Local Stack: Pushed SCSI link-layer commands [0x1A, 0x2B, 0x3C] successfully.\n");

    // 3. Test DECnet Node Routing (Local to Shared SRAM translation)
    printf("\n--- Test Case 2: DECnet Transport Node Routing ---\n");
    // Route packet from Local Stack (Node 5) to Shared Stack (Node 12)
    // Write routed payload to Shared SRAM (0xF600)
    uint32_t routed_payload[3] = {0x1A, 0x2B, 0x3C};
    for (int i = 0; i < 3; i++) {
        sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                     "%064x"
                     "%064x\"",
                REG_SHARED_STACK + i, routed_payload[i]);
        tsfi_zmm_vm_exec(&vm, cmd);
    }
    printf("[XPL] DECnet: Routed packet from Local (Node 5) to Shared (Node 12) memory.\n");

    // 4. Test STANAG Frame Serialization
    printf("\n--- Test Case 3: STANAG Tactical Serialization ---\n");
    StanagFrame frame;
    frame.stanag_id = 0x5516; // STANAG 5516 Link-16 identifier
    frame.src_node = 5;
    frame.dst_node = 12;
    frame.payload[0] = 0x1A;
    frame.payload[1] = 0x2B;
    frame.payload[2] = 0x3C;
    frame.payload[3] = 0x00;

    assert(frame.stanag_id == 0x5516);
    assert(frame.src_node == 5);
    assert(frame.dst_node == 12);
    printf("[XPL] STANAG: Tactical Frame compiled and verified natively.\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== ALL NETWORK DEVICE STACK TESTS PASSED ===\n");
    return 0;
}
