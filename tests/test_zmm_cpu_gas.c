#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

// Mock CPU 6502 state
typedef struct {
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t sp;
    uint8_t sr;
    uint16_t pc;
    uint64_t gas_balance;
    uint64_t gas_consumed;
} Cpu6502State;

// Mock Folklore CPU state (mapped to the 9-epoch sequence)
typedef struct {
    uint32_t epoch;
    uint64_t base;
    uint64_t secret;
    uint64_t signal;
    uint64_t gas_balance;
    uint64_t gas_consumed;
} FolkloreCpuState;

// Opcode gas weights for 6502 CPU
uint32_t get_6502_opcode_gas_cost(uint8_t opcode) {
    switch (opcode) {
        case 0xA9: return 3; // LDA Immediate
        case 0x69: return 4; // ADC Immediate
        case 0x8D: return 4; // STA Absolute
        case 0x00: return 7; // BRK
        default:   return 2; // Default baseline cost
    }
}

// Opcode gas weights for Folklore CPU
uint32_t get_folklore_epoch_gas_cost(uint32_t epoch) {
    switch (epoch) {
        case 0: return 10; // INIT Epoch
        case 2: return 15; // FORM Epoch (Uses ModPow)
        case 5: return 15; // CONIFY Epoch (Uses ModPow)
        case 8: return 20; // MAGNETIZE Epoch (Final convergence)
        default: return 8; // Intermediate transition step cost
    }
}

// Simulate 6502 execution step
bool step_6502(Cpu6502State *cpu, uint8_t opcode, const char **out_err) {
    uint32_t cost = get_6502_opcode_gas_cost(opcode);
    
    if (cpu->gas_balance < cost) {
        *out_err = "CPU_OUT_OF_GAS: 6502 instruction execution aborted";
        return false;
    }
    
    cpu->gas_balance -= cost;
    cpu->gas_consumed += cost;
    cpu->pc += (opcode == 0x8D) ? 3 : (opcode == 0x00) ? 1 : 2; // Simple PC increment
    
    printf("   [6502 Step] PC: 0x%04X | Opcode: 0x%02X | Gas Cost: %d | Remaining Gas: %lu\n",
           cpu->pc, opcode, cost, cpu->gas_balance);
    return true;
}

// Simulate Folklore CPU execution step
bool step_folklore(FolkloreCpuState *cpu, const char **out_err) {
    uint32_t cost = get_folklore_epoch_gas_cost(cpu->epoch);
    
    if (cpu->gas_balance < cost) {
        *out_err = "CPU_OUT_OF_GAS: Folklore epoch transition aborted";
        return false;
    }
    
    cpu->gas_balance -= cost;
    cpu->gas_consumed += cost;
    
    printf("   [Folklore Step] Transitioning Epoch %d -> %d | Gas Cost: %d | Remaining Gas: %lu\n",
           cpu->epoch, cpu->epoch + 1, cost, cpu->gas_balance);
           
    cpu->epoch++;
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: 6502 & FOLKLORE CPU GAS USAGE TESTS\n");
    printf("=============================================================\n");

    const char *err = NULL;

    // --- TEST 1: CPU 6502 Gas Allocation and Tracking ---
    printf("1. Initializing 6502 CPU with 25 Gas tokens...\n");
    Cpu6502State cpu6502 = {
        .a = 0, .x = 0, .y = 0, .sp = 0xFF, .sr = 0x20, .pc = 0x0600,
        .gas_balance = 25,
        .gas_consumed = 0
    };

    // LDA #$05 (Opcode A9)
    bool ok = step_6502(&cpu6502, 0xA9, &err);
    assert(ok);
    
    // ADC #$03 (Opcode 69)
    ok = step_6502(&cpu6502, 0x69, &err);
    assert(ok);
    
    // STA $0200 (Opcode 8D)
    ok = step_6502(&cpu6502, 0x8D, &err);
    assert(ok);
    
    // BRK (Opcode 00)
    ok = step_6502(&cpu6502, 0x00, &err);
    assert(ok);

    assert(cpu6502.gas_consumed == 18); // 3 (LDA) + 4 (ADC) + 4 (STA) + 7 (BRK)
    assert(cpu6502.gas_balance == 7);
    printf("   ✓ 6502 Execution Passed. Total Gas Consumed: %lu\n\n", cpu6502.gas_consumed);

    // --- TEST 2: CPU 6502 Out of Gas Rejection ---
    printf("2. Testing 6502 Out of Gas Rejection (Bal: 5, Op: BRK, Cost: 7)...\n");
    cpu6502.gas_balance = 5;
    ok = step_6502(&cpu6502, 0x00, &err);
    assert(!ok);
    assert(strcmp(err, "CPU_OUT_OF_GAS: 6502 instruction execution aborted") == 0);
    printf("   ✓ Reversion Successful: %s\n\n", err);

    // --- TEST 3: Folklore CPU Gas Allocation and Tracking ---
    printf("3. Initializing Folklore CPU with 120 Gas tokens...\n");
    FolkloreCpuState folklore = {
        .epoch = 0,
        .base = 12345,
        .secret = 67890,
        .signal = 55555,
        .gas_balance = 120,
        .gas_consumed = 0
    };

    // Execute standard 9-epoch transition chain
    for (int i = 0; i < 9; i++) {
        ok = step_folklore(&folklore, &err);
        assert(ok);
    }
    
    // Total Expected Gas = 10 (Init) + 8 (Avail) + 15 (Form) + 8 (Polarize) + 8 (Conjugate)
    //                      + 15 (Conify) + 8 (Saturate) + 8 (Ionize) + 20 (Magnetize) = 100
    assert(folklore.gas_consumed == 100);
    assert(folklore.gas_balance == 20);
    printf("   ✓ Folklore Execution Passed. Total Gas Consumed: %lu\n\n", folklore.gas_consumed);

    // --- TEST 4: Folklore Out of Gas Rejection ---
    printf("4. Testing Folklore Out of Gas Rejection (Bal: 10, Target: Magnetize [Cost: 20])...\n");
    folklore.epoch = 8; // Magnetize Epoch
    folklore.gas_balance = 10;
    ok = step_folklore(&folklore, &err);
    assert(!ok);
    assert(strcmp(err, "CPU_OUT_OF_GAS: Folklore epoch transition aborted") == 0);
    printf("   ✓ Reversion Successful: %s\n\n", err);

    printf("=============================================================\n");
    printf("AUNCIENT CPU GAS USAGE TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
