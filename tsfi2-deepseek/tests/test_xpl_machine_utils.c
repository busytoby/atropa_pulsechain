#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define REG_VCE_BASE 62208   // 0xF300
#define REG_SCSI_PHASE 0xFB00 // 64256
#define REG_PSG_BASE 0xF100   // 61696

int main() {
    printf("=== TSFi ZMM VM Auncient XPL Machine Utilities Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and deploy hucSystem.yul
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char cmd[512];

    // 2. Test MEM_DUMP utility: Write to VCE register index 0 and verify
    printf("\n--- Test Case 1: MEM_DUMP ---\n");
    // Write 306 (0x132) to VCE index 0
    sprintf(cmd, "YULEXEC \"huc\", \"c14ab90d"
                 "%064x"
                 "%064x"
                 "%064x"
                 "%064x\"",
            0, 4, 6, 2);
    tsfi_zmm_vm_exec(&vm, cmd);

    // Read back VCE index 0
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"huc\", \"22bbfa840000000000000000000000000000000000000000000000000000000000000000\"");
    printf("[ZMM] VCE Color 0: %s\n", vm.output_buffer);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000132") != NULL);
    printf("[XPL] MEM_DUMP: Successfully read and verified register values.\n");

    // 3. Test SCSI_SCAN utility: Execute Selection -> Data In -> Bus Free
    printf("\n--- Test Case 2: SCSI_SCAN Loop ---\n");
    // Initialize SCSI Phase to 1 (Selection)
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_SCSI_PHASE, 1);
    tsfi_zmm_vm_exec(&vm, cmd);

    // Check Phase is 1
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"huc\", \"1fca8099\""); // cdStat/general read hook
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000001") != NULL);
    printf("[XPL] SCSI_SCAN: Transition to Selection Phase successful.\n");

    // 4. Test PSG_WAVE utility: Write square wave to channel 0 (offset 0xF100)
    printf("\n--- Test Case 3: PSG_WAVE wave Setup ---\n");
    // Write alternating 15 and 0 values to the first 16 steps of Channel 0
    for (int i = 0; i < 16; i++) {
        unsigned int val = (i < 8) ? 15 : 0;
        sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                     "%064x"
                     "%064x\"",
                REG_PSG_BASE + i, val);
        tsfi_zmm_vm_exec(&vm, cmd);
    }
    printf("[XPL] PSG_WAVE: Square wave parameters written successfully to 0xF100.\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== ALL XPL MACHINE UTILITIES TESTS PASSED ===\n");
    return 0;
}
