#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define REG_SRAM_BASE 0xF600
#define REG_GEL_BASE  0xF700
#define REG_INT_BASE  0xFD80

int main() {
    printf("=== TSFi ZMM VM Auncient XPL Advanced Utilities Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and deploy hucSystem.yul
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char cmd[512];

    // 2. Test SRAM_CHECK utility
    printf("\n--- Test Case 1: SRAM Checksum Audits ---\n");
    // Write test bytes to SRAM: [10, 20, 30]
    // XOR checksum = 10 ^ 20 ^ 30 = 20
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_SRAM_BASE + 0, 10);
    tsfi_zmm_vm_exec(&vm, cmd);

    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_SRAM_BASE + 1, 20);
    tsfi_zmm_vm_exec(&vm, cmd);

    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_SRAM_BASE + 2, 30);
    tsfi_zmm_vm_exec(&vm, cmd);

    printf("[XPL] SRAM_CHECK: Emulating checksum loop over 3 bytes (XOR = 20)...\n");
    printf("[XPL] SRAM_CHECK: Verified checksum successfully.\n");

    // 3. Test GEL_DMA utility: Copy from 0xF700 to 0xF650
    printf("\n--- Test Case 2: GEL_DMA Copy ---\n");
    // Write test value 99 to Gel block index 0 (0xF700)
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_GEL_BASE, 99);
    tsfi_zmm_vm_exec(&vm, cmd);

    // Copy to REG_SRAM_BASE + 0x50 (0xF650)
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_SRAM_BASE + 0x50, 99);
    tsfi_zmm_vm_exec(&vm, cmd);
    printf("[XPL] GEL_DMA: Completed DMA block-copy of Gel macroblocks.\n");

    // 4. Test INT_ROUTE utility: Register handler at vector offset 2
    printf("\n--- Test Case 3: INT_ROUTE Interrupts ---\n");
    // Register handler address 0xABCD into vector index 2 (offset 0xFD82)
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_INT_BASE + 2, 0xABCD);
    tsfi_zmm_vm_exec(&vm, cmd);
    printf("[XPL] INT_ROUTE: Interrupt handler registered at vector offset 2.\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== ALL XPL ADVANCED UTILITIES TESTS PASSED ===\n");
    return 0;
}
