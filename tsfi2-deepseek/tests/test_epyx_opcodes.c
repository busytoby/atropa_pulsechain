#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

// Helper to format a 256-bit argument (32 bytes) for EVM calldata
void format_uint256_hex(char *dest, uint64_t val) {
    for (int i = 0; i < 32; i++) {
        int shift = (31 - i) * 8;
        uint8_t byteval = 0;
        if (shift < 64) {
            byteval = (val >> shift) & 0xFF;
        }
        sprintf(&dest[i * 2], "%02x", byteval);
    }
}

// VM Poke Helper via YULEXEC
void vm_poke(TsfiZmmVmState *vm, uint64_t addr, uint64_t val) {
    char cmd[512];
    char addr_hex[65];
    char val_hex[65];
    format_uint256_hex(addr_hex, addr);
    format_uint256_hex(val_hex, val);
    
    // selector: poke(uint256,uint256) -> 0x8029e7c0
    sprintf(cmd, "YULEXEC \"cpu6502\", \"8029e7c0%s%s\"", addr_hex, val_hex);
    vm->output_pos = 0;
    tsfi_zmm_vm_exec(vm, cmd);
}

// VM Peek Helper via YULEXEC
uint64_t vm_peek(TsfiZmmVmState *vm, uint64_t addr) {
    char cmd[512];
    char addr_hex[65];
    format_uint256_hex(addr_hex, addr);
    
    // selector: peek(uint256) -> 0x7861d269
    sprintf(cmd, "YULEXEC \"cpu6502\", \"7861d269%s\"", addr_hex);
    vm->output_pos = 0;
    tsfi_zmm_vm_exec(vm, cmd);
    
    // Parse the 32-byte hex returned in vm->output_buffer (64 hex characters)
    uint64_t res = 0;
    // We only care about the last 8 bytes (16 hex chars) of the uint256
    size_t len = strlen(vm->output_buffer);
    if (len >= 64) {
        char val_str[17];
        strncpy(val_str, &vm->output_buffer[len - 16], 16);
        val_str[16] = '\0';
        res = strtoull(val_str, NULL, 16);
    }
    return res;
}

// VM runSteps Helper via YULEXEC
uint64_t vm_runSteps(TsfiZmmVmState *vm, uint64_t maxSteps) {
    char cmd[512];
    char steps_hex[65];
    format_uint256_hex(steps_hex, maxSteps);
    
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
    printf("=== TSFi ZMM VM Epyx Opcodes (cpu6502.yul) Integration Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Initialize Yul system for cpu6502.yul via VM text commands
    printf("[TEST 1] Initializing cpu6502 Yul CPU via ZMM VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"cpu6502\", \"../solidity/bin/cpu6502.yul\", 1");
    printf("PASS\n");

    // 2. Verify LAX zero-page (0xA7)
    // Program: LAX $10 (0xA7 0x10), BRK (0x00)
    printf("[TEST 2] Verifying LAX zero-page (0xA7) undocumented opcode...\n");
    // Reset registers
    for (int i = 0x80; i <= 0x85; i++) {
        vm_poke(&vm, i, 0);
    }
    vm_poke(&vm, 0x85, 8192); // Set PC to 8192
    vm_poke(&vm, 0x83, 0xFF); // Set SP to 0xFF

    // Poke program bytes
    vm_poke(&vm, 8192, 0xA7);
    vm_poke(&vm, 8193, 0x10);
    vm_poke(&vm, 8194, 0x00);

    // Setup zero-page address $10 with value 77
    vm_poke(&vm, 0x10, 77);

    // Execute
    vm_runSteps(&vm, 5);

    // Peek A and X
    uint64_t A = vm_peek(&vm, 0x80);
    uint64_t X = vm_peek(&vm, 0x81);
    printf("A: %lu, X: %lu\n", A, X);
    assert(A == 77 && X == 77);
    printf("PASS\n");

    // 3. Verify SAX zero-page (0x87)
    // Program: SAX $20 (0x87 0x20), BRK (0x00)
    printf("[TEST 3] Verifying SAX zero-page (0x87) undocumented opcode...\n");
    for (int i = 0x80; i <= 0x85; i++) {
        vm_poke(&vm, i, 0);
    }
    vm_poke(&vm, 0x85, 8192); // Set PC to 8192
    vm_poke(&vm, 0x83, 0xFF); // Set SP to 0xFF
    vm_poke(&vm, 0x80, 0xAA); // Set A to 0xAA
    vm_poke(&vm, 0x81, 0xF0); // Set X to 0xF0

    // Poke program bytes
    vm_poke(&vm, 8192, 0x87);
    vm_poke(&vm, 8193, 0x20);
    vm_poke(&vm, 8194, 0x00);

    // Execute
    vm_runSteps(&vm, 5);

    // Peek RAM address $20
    uint64_t ram_val = vm_peek(&vm, 0x20);
    printf("RAM[$20]: 0x%lx (Expected 0xa0)\n", ram_val);
    assert(ram_val == 0xA0);
    printf("PASS\n");

    // 4. Verify Undocumented NOPs (0x04, 0x0C)
    // Program: 0x04 0x22 (NOP zp), 0x0C 0x33 0x44 (NOP abs), 0xEA (NOP), 0x00 (BRK)
    printf("[TEST 4] Verifying Undocumented NOPs (0x04, 0x0C) and standard NOP (0xEA)...\n");
    for (int i = 0x80; i <= 0x85; i++) {
        vm_poke(&vm, i, 0);
    }
    vm_poke(&vm, 0x85, 8192); // Set PC to 8192
    vm_poke(&vm, 0x83, 0xFF); // Set SP to 0xFF

    // Poke program bytes
    vm_poke(&vm, 8192, 0x04);
    vm_poke(&vm, 8193, 0x22);
    vm_poke(&vm, 8194, 0x0C);
    vm_poke(&vm, 8195, 0x33);
    vm_poke(&vm, 8196, 0x44);
    vm_poke(&vm, 8197, 0xEA);
    vm_poke(&vm, 8198, 0x00);

    // Execute
    vm_runSteps(&vm, 10);

    // Peek PC (PC should stop at 8198)
    uint64_t PC = vm_peek(&vm, 0x85);
    printf("Final PC: %lu (Expected 8198)\n", PC);
    assert(PC == 8198);
    printf("PASS\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL ZMM VM EPYX TESTS PASSED ===\n");
    return 0;
}
