#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
    uint32_t value;
    bool parity;
} parity_reg_t;

typedef struct {
    parity_reg_t accumulator;
    parity_reg_t pc;
    bool halted;
    bool audit_fault_flag;
} protected_cpu_t;

// Calculate odd parity for a 32-bit register value
bool calculate_reg_parity(uint32_t val) {
    uint32_t count = 0;
    for (int i = 0; i < 32; i++) {
        if ((val >> i) & 1) {
            count++;
        }
    }
    return (count % 2) != 0;
}

// Write to a parity-protected register
void write_protected_reg(parity_reg_t *reg, uint32_t val) {
    reg->value = val;
    reg->parity = calculate_reg_parity(val);
}

// Read and audit a parity-protected register
bool read_and_audit_reg(const parity_reg_t *reg, uint32_t *out_val) {
    bool calculated = calculate_reg_parity(reg->value);
    if (calculated != reg->parity) {
        return false; // Audit check failed (tampering or hardware fault)
    }
    *out_val = reg->value;
    return true;
}

// Execute a step on the protected CPU
void step_protected_cpu(protected_cpu_t *cpu) {
    uint32_t current_pc = 0;
    uint32_t current_acc = 0;

    // Audit the program counter and accumulator before execution
    if (!read_and_audit_reg(&cpu->pc, &current_pc) || 
        !read_and_audit_reg(&cpu->accumulator, &current_acc)) {
        cpu->audit_fault_flag = true;
        cpu->halted = true;
        printf("   [AUDIT HALT] Unified Register Parity Audit failed! Tampering detected.\n");
        return;
    }

    // Simple execution simulation: Increment PC
    write_protected_reg(&cpu->pc, current_pc + 1);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT UNIFIED REGISTER PARITY AUDIT SUITE\n");
    printf("=============================================================\n");

    protected_cpu_t cpu;
    memset(&cpu, 0, sizeof(cpu));

    // Initialize registers normally
    write_protected_reg(&cpu.accumulator, 100);
    write_protected_reg(&cpu.pc, 10);
    cpu.halted = false;
    cpu.audit_fault_flag = false;


    // 1. Normal execution sequence
    printf("[TEST] Running normal CPU execution cycles...\n");
    step_protected_cpu(&cpu);
    assert(!cpu.audit_fault_flag);
    assert(cpu.pc.value == 11);
    printf("   ✓ Normal cycle passed parity audits. PC updated to 11.\n");

    // 2. Simulating an adversarial POKE attack on the Program Counter
    printf("[TEST] Simulating adversarial POKE attack on the Program Counter...\n");
    // Adversary bypasses write_protected_reg and modifies the raw value directly
    cpu.pc.value = 999; // raw write without updating parity bit
    
    step_protected_cpu(&cpu);
    assert(cpu.audit_fault_flag == true);
    assert(cpu.halted == true);
    printf("   ✓ POKE attack successfully blocked. CPU halted under parity audit.\n");

    // 3. Repairing and recovering CPU registers
    printf("[TEST] Restoring and repairing CPU state...\n");
    write_protected_reg(&cpu.pc, 12);
    cpu.audit_fault_flag = false;
    cpu.halted = false;

    step_protected_cpu(&cpu);
    assert(!cpu.audit_fault_flag);
    assert(cpu.pc.value == 13);
    printf("   ✓ CPU state repaired and execution resumed successfully.\n");

    printf("=============================================================\n");
    printf("UNIFIED REGISTER PARITY AUDIT TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
