#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

int main() {
    printf("=== TSFi C128 Shadow Register Simulation Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Initialize CPU6502 Yul contract
    printf("[TEST] Initializing CPU6502 contract...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"cpu6502\", \"../solidity/bin/cpu6502.yul\", 1");

    // 2. Load 6502 assembly program demonstrating Shadow Registers:
    // Write 0x05 to the Shadow Register for Border Color (address $0FA0 / 4000)
    // Run interrupt handler which reads $0FA0 and copies it to hardware border register $D020 (53280)
    // Program:
    // LDA #$05
    // STA 4000     (Write to shadow register $0FA0)
    // JSR $C000    (Simulated interrupt sync routine)
    // BRK
    // Interrupt sync routine at $C000:
    // LDA 4000
    // STA 53280    (Sync to active hardware register $D020)
    // RTS
    
    // Write mainline program to RAM (starting at 1536 / $0600)
    // LDA #$05  (A9 05)
    // STA $0FA0 (8D A0 0F)
    // JSR $C000 (20 00 C0)
    // BRK       (00)
    uint32_t program_addrs[] = {1536, 1537, 1538, 1539, 1540, 1541, 1542, 1543, 1544};
    uint32_t program_vals[]  = {0xA9, 0x05, 0x8D, 0xA0, 0x0F, 0x20, 0x00, 0xC0, 0x00};
    int prog_len = sizeof(program_addrs) / sizeof(program_addrs[0]);

    for (int i = 0; i < prog_len; i++) {
        char cmd[512];
        sprintf(cmd, "YULEXEC \"cpu6502\", \"8029e7c0%064x%064x\"", program_addrs[i], program_vals[i]);
        tsfi_zmm_vm_exec(&vm, cmd);
    }

    // Write interrupt sync subroutine to RAM (starting at 49152 / $C000)
    // LDA $0FA0 (AD A0 0F)
    // STA $D020 (8D 20 D0)
    // RTS       (60)
    uint32_t sub_addrs[] = {49152, 49153, 49154, 49155, 49156, 49157, 49158};
    uint32_t sub_vals[]  = {0xAD,  0xA0,  0x0F,  0x8D,  0x20,  0xD0,  0x60};
    int sub_len = sizeof(sub_addrs) / sizeof(sub_addrs[0]);

    for (int i = 0; i < sub_len; i++) {
        char cmd[512];
        sprintf(cmd, "YULEXEC \"cpu6502\", \"8029e7c0%064x%064x\"", sub_addrs[i], sub_vals[i]);
        tsfi_zmm_vm_exec(&vm, cmd);
    }

    // 3. Initialize Registers (PC = 1536, SP = $FF)
    uint32_t reg_addrs[] = {128, 129, 130, 131, 132, 133};
    uint32_t reg_vals[]  = {0,   0,   0,   0xFF, 0x20, 1536};
    for (int i = 0; i < 6; i++) {
        char cmd[512];
        sprintf(cmd, "YULEXEC \"cpu6502\", \"8029e7c0%064x%064x\"", reg_addrs[i], reg_vals[i]);
        tsfi_zmm_vm_exec(&vm, cmd);
    }

    // 4. Run steps
    printf("[TEST] Running CPU program executing shadow copy sync...\n");
    char cmd[512];
    sprintf(cmd, "YULEXEC \"cpu6502\", \"c45b1808%064x\"", 30);
    tsfi_zmm_vm_exec(&vm, cmd);

    // 5. Peek destination hardware register $D020 (53280) to verify synchronization
    printf("[TEST] Peeking destination register $D020 (53280)...\n");
    sprintf(cmd, "YULEXEC \"cpu6502\", \"7861d269%064x\"", 53280);
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);

    printf("Value at $D020: %s\n", vm.output_buffer);

    const char *expected = "0000000000000000000000000000000000000000000000000000000000000005";
    assert(strcmp(vm.output_buffer, expected) == 0);
    printf("PASS: Shadow register sync verified (Value = 5 in hardware boundary register)!\n");

    tsfi_zmm_vm_destroy(&vm);
    return 0;
}
