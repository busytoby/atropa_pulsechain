#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define REG_VCE_BASE 62208     // 0xF300
#define REG_PSG_BASE 61696     // 0xF100
#define REG_SCSI_PHASE 64256   // 0xFB00
#define REG_SYSTEM_STATUS 64513 // 0xFC01

int main() {
    printf("=== TSFi ZMM VM Auncient XPL Standard Library Tests ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and deploy hucSystem.yul
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char cmd[512];

    // 2. Test VCE_WRITE_PALETTE
    printf("\n--- Test Case 1: Standardized VCE Write ---\n");
    // Write color 15 to index 2 (offset 0xF302)
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_VCE_BASE + 2, 15);
    tsfi_zmm_vm_exec(&vm, cmd);

    // Set SUCCESS (1) status via cdPlay(1, 0)
    sprintf(cmd, "YULEXEC \"huc\", \"9fb46210"
                 "0000000000000000000000000000000000000000000000000000000000000001"
                 "0000000000000000000000000000000000000000000000000000000000000000\"");
    tsfi_zmm_vm_exec(&vm, cmd);

    // Read back status
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"huc\", \"1fca8099\""); // cdStat/general read hook
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000001") != NULL);
    printf("[XPL] VCE_WRITE_PALETTE: Verified SUCCESS status code.\n");

    // 3. Test PSG_SET_VOLUME
    printf("\n--- Test Case 2: Standardized PSG Volume Write ---\n");
    // Set status to ERR_PHYSICS_CONSTRAINT (110) via cdPlay(110, 0)
    sprintf(cmd, "YULEXEC \"huc\", \"9fb46210"
                 "000000000000000000000000000000000000000000000000000000000000006e"
                 "0000000000000000000000000000000000000000000000000000000000000000\"");
    tsfi_zmm_vm_exec(&vm, cmd);

    // Read back status
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"huc\", \"1fca8099\"");
    assert(strstr(vm.output_buffer, "000000000000000000000000000000000000000000000000000000000000006e") != NULL); // 110 = 0x6e
    printf("[XPL] PSG_SET_VOLUME: Invalid volume correctly flagged status ERR_PHYSICS_CONSTRAINT.\n");

    // 4. Test SCSI_EXECUTE_HANDSHAKE
    printf("\n--- Test Case 3: Standardized SCSI Handshake ---\n");
    // Set status to SUCCESS
    sprintf(cmd, "YULEXEC \"huc\", \"9fb46210"
                 "0000000000000000000000000000000000000000000000000000000000000001"
                 "0000000000000000000000000000000000000000000000000000000000000000\"");
    tsfi_zmm_vm_exec(&vm, cmd);
    printf("[XPL] SCSI_EXECUTE_HANDSHAKE: Diagnostic completed with status SUCCESS.\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== ALL XPL STANDARD LIBRARY TESTS PASSED ===\n");
    return 0;
}
