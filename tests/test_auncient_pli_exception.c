#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define ACC_MAX_LIMIT 100
#define EXCEPTION_HANDLER_PC 9000

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// PL/I-style ALU with ON-Condition hardware gating
typedef struct {
    uint32_t PC;
    uint64_t ACC;
    uint32_t on_overflow_handler;
    gate_state_t exception_gate;
} pli_alu_t;

// -------------------------------------------------------------
// PL/I ON-Overflow hardware gating execution step
// -------------------------------------------------------------
void pli_execute_step(pli_alu_t *alu, uint32_t val) {
    uint64_t next_acc = alu->ACC + val;

    // Simulate PL/I ON OVERFLOW hardware comparator
    if (next_acc > ACC_MAX_LIMIT) {
        // Trigger condition handler: route PC directly to registered handler address
        alu->PC = alu->on_overflow_handler;
        alu->exception_gate = CONDUC_STATE;
    } else {
        // Normal execution path
        alu->ACC = next_acc;
        alu->PC++;
        alu->exception_gate = CUTOFF_STATE;
    }
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PL/I ON-CONDITION HARDWARE GATE VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    pli_alu_t alu = {
        .PC = 1000,
        .ACC = 40,
        .on_overflow_handler = EXCEPTION_HANDLER_PC,
        .exception_gate = CUTOFF_STATE
    };

    // 1. Execute normal instruction (ACC 40 + 30 = 70 <= 100) -> Should update ACC and advance PC
    printf("[TEST] Executing normal instruction (No overflow)...\n");
    fflush(stdout);
    pli_execute_step(&alu, 30);
    assert(alu.PC == 1001);
    assert(alu.ACC == 70);
    assert(alu.exception_gate == CUTOFF_STATE);
    printf("   ✓ Instruction executed successfully. PC advanced.\n");
    fflush(stdout);

    // 2. Trigger ON OVERFLOW condition (ACC 70 + 40 = 110 > 100) -> Should route to exception handler
    printf("[TEST] Executing overflow instruction (Triggers ON OVERFLOW)...\n");
    fflush(stdout);
    pli_execute_step(&alu, 40);
    assert(alu.PC == EXCEPTION_HANDLER_PC); // Routed directly to registered handler address
    assert(alu.ACC == 70); // State preserved
    assert(alu.exception_gate == CONDUC_STATE); // Exception routing active
    printf("   ✓ Hardware exception comparator routed directly to ON-condition handler.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("PL/I ON-CONDITION HARDWARE GATE TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
