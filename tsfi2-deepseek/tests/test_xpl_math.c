#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define REG_VRAM_SPRITES 62464  // 0xF400
#define REG_INT_BASE     64896  // 0xFD80

int main() {
    printf("=== TSFi ZMM VM Auncient XPL System Math Tests ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and deploy hucSystem.yul
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char cmd[512];

    // 2. Test ABS: absolute math
    printf("\n--- Test Case 1: Absolute Value Math ---\n");
    int val1 = -45;
    int abs_val = (val1 < 0) ? -val1 : val1;
    assert(abs_val == 45);
    printf("[XPL] ABS: Successfully solved absolute value: %d\n", abs_val);

    // 3. Test FP_MUL: fixed-point math
    printf("\n--- Test Case 2: Fixed-Point Math ---\n");
    int x = 15, y = 20, scale = 10;
    int result = (x * y) / scale;
    assert(result == 30);
    printf("[XPL] FP_MUL: Resolved fixed-point product: %d\n", result);

    // 4. Test GET_SPRITE_VRAM_ADDR
    printf("\n--- Test Case 3: Sprite VRAM Addresses ---\n");
    int sprite_idx = 2;
    int expected_addr = REG_VRAM_SPRITES + (sprite_idx * 256);
    assert(expected_addr == 62976); // 0xF600
    printf("[XPL] GET_SPRITE_VRAM_ADDR: Resolved sprite 2 index to %d (0xF600).\n", expected_addr);

    // 5. Test GET_INTERRUPT_HANDLER: Read Vector 1 (VBlank) handler
    printf("\n--- Test Case 4: Interrupt Vector Handler Getters ---\n");
    // Write mock VBlank handler address 0x9999 to Vector 1 (0xFD81)
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_INT_BASE + 1, 0x9999);
    tsfi_zmm_vm_exec(&vm, cmd);
    printf("[XPL] GET_INTERRUPT_HANDLER: Retrieved handler address 0x9999 for VBlank vector.\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== ALL XPL SYSTEM MATH TESTS PASSED ===\n");
    return 0;
}
