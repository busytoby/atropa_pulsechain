#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// Folklore CPU Registers
typedef struct {
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint16_t pc;
    uint8_t sp;
    uint8_t sr;
} folklore_cpu_t;

// 64KB virtual RAU memory block
static uint8_t virtual_rau_mem[65536];

// Initialize CPU state
static void folklore_cpu_init(folklore_cpu_t *cpu) {
    cpu->a = 0;
    cpu->x = 0;
    cpu->y = 0;
    cpu->pc = 0x0200;
    cpu->sp = 0xFF;
    cpu->sr = 0;
    memset(virtual_rau_mem, 0, sizeof(virtual_rau_mem));
}

// 1. RED ANALYZER Gating: Static instruction set audit
static bool red_analyzer_audit_opcode(uint8_t opcode) {
    // Whitelisted opcodes: 0xA9 (LDA imm), 0xAD (LDA abs), 0x8D (STA abs), 0x69 (ADC imm), 0x4C (JMP)
    if (opcode != 0xA9 && opcode != 0xAD && opcode != 0x8D && opcode != 0x69 && opcode != 0x4C) {
        return false; // Prohibited opcode (RED rail block)
    }
    return true;
}

// 2. BLACK XPLSM Gating: Dynamic memory boundary check
static bool black_xplsm_check_mem_access(uint32_t address, uint32_t *interrupt_out) {
    // 64KB RAM allocation bounds check
    if (address > 0xFFFF) {
        if (interrupt_out) *interrupt_out = 25; // Trap 25: Memory Range Overflow
        return false; // BLACK rail block
    }
    return true;
}

// Execute single clock step
static bool folklore_cpu_step(folklore_cpu_t *cpu, uint32_t *interrupt_out) {
    if (!black_xplsm_check_mem_access(cpu->pc, interrupt_out)) {
        return false; // PC out of bounds
    }

    uint8_t opcode = virtual_rau_mem[cpu->pc];
    if (!red_analyzer_audit_opcode(opcode)) {
        return false; // Illegal opcode execution halted
    }

    switch (opcode) {
        case 0xA9: { // LDA Immediate
            uint16_t arg_addr = cpu->pc + 1;
            if (!black_xplsm_check_mem_access(arg_addr, interrupt_out)) return false;
            cpu->a = virtual_rau_mem[arg_addr];
            cpu->pc += 2;
            break;
        }
        case 0xAD: { // LDA Absolute
            uint16_t arg_addr = cpu->pc + 1;
            if (!black_xplsm_check_mem_access(arg_addr, interrupt_out)) return false;
            uint16_t target_addr = virtual_rau_mem[arg_addr];
            if (!black_xplsm_check_mem_access(target_addr, interrupt_out)) return false;
            cpu->a = virtual_rau_mem[target_addr];
            cpu->pc += 2;
            break;
        }
        case 0x8D: { // STA Absolute
            uint16_t arg_addr = cpu->pc + 1;
            if (!black_xplsm_check_mem_access(arg_addr, interrupt_out)) return false;
            uint16_t target_addr = virtual_rau_mem[arg_addr];
            if (!black_xplsm_check_mem_access(target_addr, interrupt_out)) return false;
            virtual_rau_mem[target_addr] = cpu->a;
            cpu->pc += 2;
            break;
        }
        case 0x69: { // ADC Immediate
            uint16_t arg_addr = cpu->pc + 1;
            if (!black_xplsm_check_mem_access(arg_addr, interrupt_out)) return false;
            cpu->a += virtual_rau_mem[arg_addr];
            cpu->pc += 2;
            break;
        }
        case 0x4C: { // JMP Absolute
            uint16_t arg_addr = cpu->pc + 1;
            if (!black_xplsm_check_mem_access(arg_addr, interrupt_out)) return false;
            uint16_t target_addr = virtual_rau_mem[arg_addr];
            cpu->pc = target_addr;
            break;
        }
    }
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPLOS FOLKLORE CPU DEPLOYMENT SUITE\n");
    printf("=============================================================\n");

    folklore_cpu_t cpu;
    folklore_cpu_init(&cpu);
    uint32_t trap_code = 0;

    // Load simple test program to virtual RAM starting at PC = 0x0200:
    // 0x0200: 0xA9 0x0A  -> LDA #10 (Load A with 10)
    // 0x0202: 0x69 0x05  -> ADC #5  (Add 5 to A)
    // 0x0204: 0x8D 0x10  -> STA $10 (Store A at RAM address 0x0010)
    // 0x0206: 0x4C 0x00  -> JMP $00 (Jump back to 0x0200)
    virtual_rau_mem[0x0200] = 0xA9;
    virtual_rau_mem[0x0201] = 0x0A;
    virtual_rau_mem[0x0202] = 0x69;
    virtual_rau_mem[0x0203] = 0x05;
    virtual_rau_mem[0x0204] = 0x8D;
    virtual_rau_mem[0x0205] = 0x10;
    virtual_rau_mem[0x0206] = 0x4C;
    virtual_rau_mem[0x0207] = 0x00; // note jump target address low byte is 0x00

    // Test 1: Step 1 - LDA Immediate
    printf("[TEST] Executing Step 1: LDA Immediate...\n");
    assert(folklore_cpu_step(&cpu, &trap_code) == true);
    assert(cpu.a == 10);
    assert(cpu.pc == 0x0202);

    // Test 2: Step 2 - ADC Immediate
    printf("[TEST] Executing Step 2: ADC Immediate...\n");
    assert(folklore_cpu_step(&cpu, &trap_code) == true);
    assert(cpu.a == 15);
    assert(cpu.pc == 0x0204);

    // Test 3: Step 3 - STA Absolute
    printf("[TEST] Executing Step 3: STA Absolute...\n");
    assert(folklore_cpu_step(&cpu, &trap_code) == true);
    assert(virtual_rau_mem[0x0010] == 15);
    assert(cpu.pc == 0x0206);

    // Test 4: Step 4 - JMP Absolute
    printf("[TEST] Executing Step 4: JMP Absolute...\n");
    assert(folklore_cpu_step(&cpu, &trap_code) == true);
    assert(cpu.pc == 0x0000); // Jumped to 0x0000

    // Test 5: RED rail illegal opcode check
    printf("[TEST] Verifying RED rail opcode validation...\n");
    virtual_rau_mem[0x0000] = 0xFF; // Prohibited opcode
    assert(folklore_cpu_step(&cpu, &trap_code) == false); // HALTED
    printf("   ✓ RED rail successfully halted execution on illegal instruction.\n");

    // Test 6: BLACK rail memory bounds overflow check
    printf("[TEST] Verifying BLACK rail memory boundary checks...\n");
    cpu.pc = 0xFFFF;
    virtual_rau_mem[0xFFFF] = 0xA9; // LDA Imm, needs byte at 0x10000 (Out of bounds)
    trap_code = 0;
    assert(folklore_cpu_step(&cpu, &trap_code) == false);
    assert(trap_code == 25); // Trap 25: Memory overflow
    printf("   ✓ BLACK rail successfully trapped memory out of bounds access (Trap 25).\n");

    printf("=============================================================\n");
    printf("ALL FOLKLORE CPU INTEGRATION TESTS COMPLETED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
