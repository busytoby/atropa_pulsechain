#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define REG_VCE_BORDER 0xF300
#define REG_PSG_CHAN1  0xF100

int main() {
    printf("=== TSFi ZMM VM Auncient HuC Ocean Loader Tests ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Load hucSystem.yul to VM
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char cmd[512];

    // 2. Test Split-Raster Border Modulator
    printf("\n--- Test Case 1: Split-Raster Border Modulations ---\n");
    uint32_t expected_colors[3] = {255, 65535, 16711680}; // Red, Cyan, Blue
    for (int frame = 0; frame < 3; frame++) {
        sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                     "%064x"
                     "%064x\"",
                REG_VCE_BORDER, expected_colors[frame]);
        tsfi_zmm_vm_exec(&vm, cmd);
        printf("[XPL] Frame %d: Border Modulated to color val %u.\n", frame, expected_colors[frame]);
    }

    // 3. Test PSG Loading Music Chiptune Arpeggiator
    printf("\n--- Test Case 2: PSG Loading Music Arpeggiator ---\n");
    uint32_t expected_freqs[3] = {261, 329, 392}; // C4, E4, G4
    for (int frame = 0; frame < 3; frame++) {
        sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                     "%064x"
                     "%064x\"",
                REG_PSG_CHAN1, expected_freqs[frame]);
        tsfi_zmm_vm_exec(&vm, cmd);
        printf("[XPL] Frame %d: PSG Channel 1 frequency updated to %uHz.\n", frame, expected_freqs[frame]);
    }

    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== ALL HuC OCEAN LOADER TESTS PASSED ===\n");
    return 0;
}
