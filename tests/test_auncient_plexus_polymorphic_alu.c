#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Tag definitions
#define TAG_DATA 0x01
#define TAG_CAPABILITY 0x02

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Operand structure representing tagged memory
typedef struct {
    uint8_t tag;
    uint64_t value;
    uint64_t bound_limit; // Only applicable for TAG_CAPABILITY
} plexus_operand_t;

// Polymorphic ALU State
typedef struct {
    gate_state_t write_enable_gate;
    uint64_t alu_output;
} plexus_alu_t;

// -------------------------------------------------------------
// PL/EXUS Tag-Polymorphic ALU Operations
// -------------------------------------------------------------
void plexus_alu_add(plexus_alu_t *alu, const plexus_operand_t *op1, const plexus_operand_t *op2) {
    alu->write_enable_gate = CUTOFF_STATE;
    alu->alu_output = 0;

    // Case 1: Both operands are raw data (Standard integer addition)
    if (op1->tag == TAG_DATA && op2->tag == TAG_DATA) {
        alu->write_enable_gate = CONDUC_STATE;
        alu->alu_output = op1->value + op2->value;
    }
    // Case 2: Op1 is Capability (Pointer) and Op2 is Data (Displacement)
    else if (op1->tag == TAG_CAPABILITY && op2->tag == TAG_DATA) {
        uint64_t target_addr = op1->value + op2->value;
        // Bounded checking: conduction occurs only if new address stays within bounds limit
        if (target_addr <= op1->bound_limit) {
            alu->write_enable_gate = CONDUC_STATE;
            alu->alu_output = target_addr;
        }
    }
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PL/EXUS POLYMORPHIC ALU VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    plexus_alu_t alu;

    // 1. Test raw data addition (Should succeed)
    printf("[TEST] Adding two integer data operands...\n");
    fflush(stdout);
    plexus_operand_t d1 = { TAG_DATA, 100, 0 };
    plexus_operand_t d2 = { TAG_DATA, 200, 0 };
    
    plexus_alu_add(&alu, &d1, &d2);
    assert(alu.write_enable_gate == CONDUC_STATE);
    assert(alu.alu_output == 300);
    printf("   ✓ Data addition completed: %lu\n", alu.alu_output);
    fflush(stdout);

    // 2. Test capability addition within bounds (Should succeed)
    printf("[TEST] Adding data displacement to pointer capability (within bounds)...\n");
    fflush(stdout);
    plexus_operand_t cap = { TAG_CAPABILITY, 0x1000, 0x1080 }; // Pointer at 0x1000, bound limit at 0x1080
    plexus_operand_t displacement_ok = { TAG_DATA, 0x20, 0 }; // Displacement of 0x20
    
    plexus_alu_add(&alu, &cap, &displacement_ok);
    assert(alu.write_enable_gate == CONDUC_STATE);
    assert(alu.alu_output == 0x1020);
    printf("   ✓ Capability displacement applied within bounds: 0x%lx\n", alu.alu_output);
    fflush(stdout);

    // 3. Test capability addition exceeding bounds (Should fail/cutoff)
    printf("[TEST] Adding data displacement exceeding pointer bounds...\n");
    fflush(stdout);
    plexus_operand_t displacement_bad = { TAG_DATA, 0x90, 0 }; // Displacement of 0x90 (Total 0x1090 > 0x1080)
    
    plexus_alu_add(&alu, &cap, &displacement_bad);
    assert(alu.write_enable_gate == CUTOFF_STATE);
    assert(alu.alu_output == 0); // Blocked
    printf("   ✓ Out-of-bounds capability displacement blocked successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("PL/EXUS POLYMORPHIC ALU TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
