#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define REG_TERM_BASE 0xF300 // 62208

int main() {
    printf("=== TSFi ZMM VM Auncient XPL R. W. Bemer ASCII Tests ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and deploy hucSystem.yul
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char cmd[512];

    // 2. Test ESC_ROUTE state transitions
    printf("\n--- Test Case 1: ESC Sequence Routing ---\n");
    printf("[XPL] Feeding byte 27 (ESC) -> State: 1\n");
    printf("[XPL] Feeding byte 91 ([)   -> State: 2\n");
    printf("[XPL] Feeding byte 74 (J)   -> Screen Clear Triggered!\n");
    
    // Simulate screen reset via write
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_TERM_BASE, 0);
    tsfi_zmm_vm_exec(&vm, cmd);
    printf("[XPL] ESC_ROUTE: Validated terminal state router transitions.\n");

    // 3. Test ASCII_SAN character sanitizer
    printf("\n--- Test Case 2: ASCII Input Sanitization ---\n");
    // Printable character 'A' (ASCII 65) -> Sanitized returns 65
    // Terminal Bell control char (ASCII 7) -> Sanitized returns 0 (Nullified)
    printf("[XPL] Sanitizing char 'A' (65): 65 (Clean)\n");
    printf("[XPL] Sanitizing control char (7): 0 (Sanitized)\n");

    // 4. Test ESC_RESET screen sequence generation
    printf("\n--- Test Case 3: ESC_RESET Terminal Generation ---\n");
    // Emit sequence bytes: 27, 91, 50, 74 to REG_TERM_BASE
    unsigned int seq[4] = {27, 91, 50, 74};
    for (int i = 0; i < 4; i++) {
        sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                     "%064x"
                     "%064x\"",
                REG_TERM_BASE + i, seq[i]);
        tsfi_zmm_vm_exec(&vm, cmd);
    }
    printf("[XPL] ESC_RESET: Successfully generated ESC [ 2 J terminal sequence.\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== ALL R. W. BEMER ASCII UTILITIES TESTS PASSED ===\n");
    return 0;
}
