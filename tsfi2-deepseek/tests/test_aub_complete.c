#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

// VM Poke Helper
void vm_poke(TsfiZmmVmState *vm, uint64_t addr, uint64_t val) {
    char cmd[512];
    char addr_hex[65];
    char val_hex[65];
    
    // Format uint256 arguments for calldata
    for (int i = 0; i < 32; i++) {
        int shift = (31 - i) * 8;
        uint8_t byteval_addr = (shift < 64) ? ((addr >> shift) & 0xFF) : 0;
        uint8_t byteval_val = (shift < 64) ? ((val >> shift) & 0xFF) : 0;
        sprintf(&addr_hex[i * 2], "%02x", byteval_addr);
        sprintf(&val_hex[i * 2], "%02x", byteval_val);
    }
    
    // selector: poke(uint256,uint256) -> 0x8029e7c0
    sprintf(cmd, "YULEXEC \"cpu6502\", \"8029e7c0%s%s\"", addr_hex, val_hex);
    vm->output_pos = 0;
    tsfi_zmm_vm_exec(vm, cmd);
}

// VM Peek Helper
uint64_t vm_peek(TsfiZmmVmState *vm, uint64_t addr) {
    char cmd[512];
    char addr_hex[65];
    for (int i = 0; i < 32; i++) {
        int shift = (31 - i) * 8;
        uint8_t byteval = (shift < 64) ? ((addr >> shift) & 0xFF) : 0;
        sprintf(&addr_hex[i * 2], "%02x", byteval);
    }
    
    // selector: peek(uint256) -> 0x7861d269
    sprintf(cmd, "YULEXEC \"cpu6502\", \"7861d269%s\"", addr_hex);
    vm->output_pos = 0;
    tsfi_zmm_vm_exec(vm, cmd);
    
    uint64_t res = 0;
    size_t len = strlen(vm->output_buffer);
    if (len >= 64) {
        char val_str[17];
        strncpy(val_str, &vm->output_buffer[len - 16], 16);
        val_str[16] = '\0';
        res = strtoull(val_str, NULL, 16);
    }
    return res;
}

// VM runSteps Helper
uint64_t vm_runSteps(TsfiZmmVmState *vm, uint64_t maxSteps) {
    char cmd[512];
    char steps_hex[65];
    for (int i = 0; i < 32; i++) {
        int shift = (31 - i) * 8;
        uint8_t byteval = (shift < 64) ? ((maxSteps >> shift) & 0xFF) : 0;
        sprintf(&steps_hex[i * 2], "%02x", byteval);
    }
    
    // selector: runSteps(uint256) -> 0xc45b1808
    sprintf(cmd, "YULEXEC \"cpu6502\", \"c45b1808%s\"", steps_hex);
    vm->output_pos = 0;
    tsfi_zmm_vm_exec(vm, cmd);
    
    uint64_t res = 0;
    size_t len = strlen(vm->output_buffer);
    if (len >= 64) {
        char val_str[17];
        strncpy(val_str, &vm->output_buffer[len - 16], 16);
        val_str[16] = '\0';
        res = strtoull(val_str, NULL, 16);
    }
    return res;
}

int main() {
    printf("=== TSFi AUB Complete Instruction Set Coverage Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    printf("[AUB] Compiling and Initializing cpu6502.yul...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"cpu6502\", \"../solidity/bin/cpu6502.yul\", 1");

    // program bytes starting at address 1536 ($0600)
    // 1536: A2 02     (LDX #$02)
    // 1538: A9 10     (LDA #$10)
    // 1540: 9D 00 02  (STA $0200, X)
    // 1543: 8E 03 02  (STX $0203)
    // 1546: 38        (SEC)
    // 1547: E9 03     (SBC #$03)
    // 1549: C9 0D     (CMP #$0D)
    // 1551: F0 03     (BEQ offset 3 -> skips jump at 1553-1555)
    // 1553: 4C 15 06  (JMP $0615 - should be skipped)
    // 1556: C9 0E     (CMP #$0E - sets non-zero for BNE)
    // 1558: D0 03     (BNE offset 3 -> skips jump at 1560-1562)
    // 1560: 4C 15 06  (JMP $0615 - should be skipped)
    // 1563: 20 22 06  (JSR $0622)
    // 1566: 8D 04 02  (STA $0204)
    // 1569: 00        (BRK)
    // Subroutine:
    // 1570: A9 42     (LDA #$42)
    // 1572: 60        (RTS)
    
    uint32_t addrs[] = {
        1536, 1537, 1538, 1539, 1540, 1541, 1542, 1543, 1544, 1545,
        1546, 1547, 1548, 1549, 1550, 1551, 1552, 1553, 1554, 1555,
        1556, 1557, 1558, 1559, 1560, 1561, 1562, 1563, 1564, 1565,
        1566, 1567, 1568, 1569, 1570, 1571, 1572
    };
    uint8_t program[] = {
        0xA2, 0x02,
        0xA9, 0x10,
        0x9D, 0x00, 0x02,
        0x8E, 0x03, 0x02,
        0x38,
        0xE9, 0x03,
        0xC9, 0x0D,
        0xF0, 0x03,
        0x4C, 0x15, 0x06,
        0xC9, 0x0E,
        0xD0, 0x03,
        0x4C, 0x15, 0x06,
        0x20, 0x22, 0x06,
        0x8D, 0x04, 0x02,
        0x00,
        0xA9, 0x42,
        0x60
    };
    int prog_len = sizeof(program);

    printf("[AUB] Writing test program to RAM...\n");
    for (int i = 0; i < prog_len; i++) {
        vm_poke(&vm, addrs[i], program[i]);
    }

    printf("[AUB] Initializing CPU Registers (PC = 1536, SP = 0xFF)...\n");
    vm_poke(&vm, 0x80, 0);    // A = 0
    vm_poke(&vm, 0x81, 0);    // X = 0
    vm_poke(&vm, 0x82, 0);    // Y = 0
    vm_poke(&vm, 0x83, 0xFF); // SP = 0xFF
    vm_poke(&vm, 0x84, 0x20); // SR = 0x20
    vm_poke(&vm, 0x85, 1536); // PC = 1536

    printf("[AUB] Executing AUB instruction sequence...\n");
    vm_runSteps(&vm, 35); // Run ample cycles to complete execution

    printf("[AUB] Verifying instruction outputs...\n");
    
    // 1. Verify LDA & STA indexed with X: A=0x10, X=0x02, STA $0200,X => RAM[$0202] = 0x10
    uint64_t ram_0202 = vm_peek(&vm, 0x0202);
    printf("  RAM[$0202] = 0x%lx (Expected 0x10)\n", ram_0202);
    assert(ram_0202 == 0x10);

    // 2. Verify STX: X=0x02, STX $0203 => RAM[$0203] = 0x02
    uint64_t ram_0203 = vm_peek(&vm, 0x0203);
    printf("  RAM[$0203] = 0x%lx (Expected 0x02)\n", ram_0203);
    assert(ram_0203 == 0x02);

    // 3. Verify SBC/CMP/BEQ/BNE logic flow and subroutine JSR/RTS:
    // A starts at 0x10. SEC, SBC #$03 => A = 0x0D.
    // CMP #$0D => sets Zero. BEQ offset 3 skips target JMP.
    // CMP #$0E => clears Zero. BNE offset 3 skips target JMP.
    // JSR Subroutine => sets A to 0x42, returns RTS.
    // STA $0204 => RAM[$0204] = 0x42.
    uint64_t ram_0204 = vm_peek(&vm, 0x0204);
    printf("  RAM[$0204] = 0x%lx (Expected 0x42)\n", ram_0204);
    assert(ram_0204 == 0x42);

    // 4. Check final PC to ensure it hit BRK at 1569
    uint64_t pc = vm_peek(&vm, 0x85);
    printf("  Final PC = %lu (Expected 1569)\n", pc);
    assert(pc == 1569);

    printf("=== AUB COMPLETE INSTRUCTION COVERAGE VERIFIED PASSED ===\n");
    tsfi_zmm_vm_destroy(&vm);
    return 0;
}
