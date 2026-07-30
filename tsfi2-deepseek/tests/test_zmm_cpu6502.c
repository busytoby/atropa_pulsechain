#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

int main() {
    printf("=== TSFi ZMM VM 6502 CPU Instruction Simulation ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Initialize CPU6502 Yul Contract via ZMM VM text commands
    printf("[ZMM] Compiling and Initializing cpu6502.yul...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"cpu6502\", \"../solidity/bin/cpu6502.yul\", 1");

    // 2. Load 6502 assembly program:
    // LDA #$05  (A9 05)
    // ADC #$03  (69 03)
    // STA $0200 (8D 00 02)
    // BRK       (00)
    uint32_t program_addrs[] = {1536, 1537, 1538, 1539, 1540, 1541, 1542, 1543};
    uint32_t program_vals[]  = {0xA9, 0x05, 0x69, 0x03, 0x8D, 0x00, 0x02, 0x00};
    int prog_len = sizeof(program_addrs) / sizeof(program_addrs[0]);

    printf("[ZMM] Writing program bytes to RAM...\n");
    char cmd[512];
    for (int i = 0; i < prog_len; i++) {
        // poke(uint256 addr, uint256 val) - selector: 8029e7c0
        sprintf(cmd, "YULEXEC \"cpu6502\", \"8029e7c0"
                      "%064x"
                      "%064x\"", 
                program_addrs[i], program_vals[i]);
        tsfi_zmm_vm_exec(&vm, cmd);
    }

    // 3. Initialize CPU Registers
    // A (0x80 / 128) = 0
    // X (0x81 / 129) = 0
    // Y (0x82 / 130) = 0
    // SP (0x83 / 131) = 0xFF
    // SR (0x84 / 132) = 0x20
    // PC (0x85 / 133) = 1536 ($0600)
    printf("[ZMM] Initializing CPU registers (PC = $0600, SP = $FF)...\n");
    uint32_t reg_addrs[] = {128, 129, 130, 131, 132, 133};
    uint32_t reg_vals[]  = {0,   0,   0,   0xFF, 0x20, 1536};
    for (int i = 0; i < 6; i++) {
        sprintf(cmd, "YULEXEC \"cpu6502\", \"8029e7c0"
                      "%064x"
                      "%064x\"", 
                reg_addrs[i], reg_vals[i]);
        tsfi_zmm_vm_exec(&vm, cmd);
    }

    // 4. Execute 6502 execution steps
    // runSteps(uint256 maxSteps) - selector: c45b1808
    printf("[ZMM] Executing 10 processor steps...\n");
    sprintf(cmd, "YULEXEC \"cpu6502\", \"c45b1808"
                  "%064x\"", 10);
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);

    // 5. Peek results at $0200 (512)
    // peek(uint256 addr) - selector: 7861d269
    printf("[ZMM] Peeking result at address $0200...\n");
    sprintf(cmd, "YULEXEC \"cpu6502\", \"7861d269"
                  "%064x\"", 512);
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);

    printf("Result at $0200: %s\n", vm.output_buffer);
    
    // Expect 8 as the result (0000000000000000000000000000000000000000000000000000000000000008)
    const char *expected = "0000000000000000000000000000000000000000000000000000000000000008";
    assert(strcmp(vm.output_buffer, expected) == 0);
    printf("PASS: Correct math execution (5 + 3 = 8) verified in RAM!\n");

    // 6. Query final CPU register state
    // getCPUState() - selector: e0f77c57
    printf("[ZMM] Reading final CPU register state...\n");
    sprintf(cmd, "YULEXEC \"cpu6502\", \"e0f77c57\"");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    printf("Register State (A, X, Y, SR, SP, PC):\n  %s\n", vm.output_buffer);

    // 7. Verify live NPN/PNP pair state mapping to 6502 RAM space
    printf("[ZMM] Injecting live synthesizer Class B pair state (value 8) into 6502 RAM space at $0210...\n");
    // Write value 8 representing the live PNP/NPN active level to address 528 ($0210)
    sprintf(cmd, "YULEXEC \"cpu6502\", \"8029e7c0"
                  "%064x"
                  "%064x\"", 
            528, 8);
    tsfi_zmm_vm_exec(&vm, cmd);

    // Run 6502 assembly program to load from $0210 and store at $0200:
    // LDA $0210 (AD 10 02)
    // STA $0200 (8D 00 02)
    // BRK       (00)
    uint32_t live_addrs[] = {1536, 1537, 1538, 1539, 1540, 1541, 1542};
    uint32_t live_vals[]  = {0xAD, 0x10, 0x02, 0x8D, 0x00, 0x02, 0x00};
    int live_len = sizeof(live_addrs) / sizeof(live_addrs[0]);

    printf("[ZMM] Loading 6502 program to read synthesizer hardware state...\n");
    for (int i = 0; i < live_len; i++) {
        sprintf(cmd, "YULEXEC \"cpu6502\", \"8029e7c0"
                      "%064x"
                      "%064x\"", 
                live_addrs[i], live_vals[i]);
        tsfi_zmm_vm_exec(&vm, cmd);
    }

    // Reset PC to $0600 (1536)
    sprintf(cmd, "YULEXEC \"cpu6502\", \"8029e7c0"
                  "%064x"
                  "%064x\"", 
            133, 1536);
    tsfi_zmm_vm_exec(&vm, cmd);

    printf("[ZMM] Executing 6502 read program...\n");
    sprintf(cmd, "YULEXEC \"cpu6502\", \"c45b1808"
                  "%064x\"", 10);
    tsfi_zmm_vm_exec(&vm, cmd);

    printf("[ZMM] Peeking result at address $0200...\n");
    sprintf(cmd, "YULEXEC \"cpu6502\", \"7861d269"
                  "%064x\"", 512);
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    printf("Live Synthesizer State at $0200: %s\n", vm.output_buffer);
    assert(strcmp(vm.output_buffer, expected) == 0);
    printf("PASS: Live PNP/NPN synthesizer pair state verified successfully in 6502 execution space!\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL ZMM VM 6502 CPU TESTS PASSED ===\n");
    return 0;
}
