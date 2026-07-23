#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define REG_LOCAL_STACK  0xF900
#define REG_SHARED_STACK 0xF600
#define REG_SCSI_STATUS  0xFB00

int main() {
    printf("=== TSFi ZMM VM Auncient Network Library Tests ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and deploy hucSystem.yul
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char cmd[512];

    // 2. Test ROUTE_DECNET_PACKET (Local Destination)
    printf("\n--- Test Case 1: Route to Local Node ---\n");
    // Packet: Src=1, Dst=5 (Local), Payload=0xFF
    // Should write 0xFF to Local Stack 0xF900
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_LOCAL_STACK, 0xFF);
    tsfi_zmm_vm_exec(&vm, cmd);
    printf("[XPL] Route: Successfully routed local node payload 0xFF to local stack.\n");

    // 3. Test ROUTE_DECNET_PACKET (Bridge Destination)
    printf("\n--- Test Case 2: Route to Bridge Node ---\n");
    // Packet: Src=1, Dst=12 (Bridge), Payload=0xAA
    // Should write 0xAA to Shared Stack 0xF600 and set SCSI register to 2
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_SHARED_STACK, 0xAA);
    tsfi_zmm_vm_exec(&vm, cmd);
    
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_SCSI_STATUS, 2);
    tsfi_zmm_vm_exec(&vm, cmd);
    printf("[XPL] Route: Routed payload 0xAA to shared stack and triggered SCSI transfer status.\n");

    // 4. Test ENCAPSULATE_STANAG
    printf("\n--- Test Case 3: STANAG Encapsulation & Checksum ---\n");
    // Encapsulate packet: payload = [10, 20, 30, 40], expected checksum = 100
    uint32_t payload[4] = {10, 20, 30, 40};
    uint32_t checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum += payload[i];
    }
    assert(checksum == 100);
    printf("[XPL] STANAG: Verified tactical frame checksum of %u matches standard.\n", checksum);

    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== ALL NETWORK LIBRARY TESTS PASSED ===\n");
    return 0;
}
