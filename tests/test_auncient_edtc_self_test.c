#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Simulated EDTC Dual-Modular Redundant (DMR) Self-Checking Register
typedef struct {
    uint32_t primary_value;
    uint32_t shadow_value;
    gate_state_t commit_gate;
    bool hardware_fault_flag;
} dmr_register_t;

// -------------------------------------------------------------
// EDTC Dependable Execution & Self-Testing Gating
// -------------------------------------------------------------
void execute_dmr_operation(dmr_register_t *reg, uint32_t input_val) {
    reg->commit_gate = CUTOFF_STATE;
    reg->hardware_fault_flag = false;

    // Redundant execution paths
    reg->primary_value += input_val;
    reg->shadow_value += input_val;

    // Dependable Computing check: Outputs must be identical
    if (reg->primary_value == reg->shadow_value) {
        reg->commit_gate = CONDUC_STATE;
    } else {
        reg->hardware_fault_flag = true; // DMR mismatch detected
    }
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT EDTC SELF-CHECKING DMR SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    dmr_register_t reg = {
        .primary_value = 10,
        .shadow_value = 10,
        .commit_gate = CUTOFF_STATE,
        .hardware_fault_flag = false
    };

    // 1. Compliant Case: Normal execution -> Should match and commit
    printf("[TEST] Executing compliant DMR operation...\n");
    fflush(stdout);
    execute_dmr_operation(&reg, 5);
    assert(reg.commit_gate == CONDUC_STATE);
    assert(reg.hardware_fault_flag == false);
    assert(reg.primary_value == 15);
    printf("   ✓ DMR values match. Write committed to register.\n");
    fflush(stdout);

    // 2. Fault Case: Simulate soft error / transient bit flip in primary value -> Should mismatch
    printf("[TEST] Simulating transient bit-flip fault in primary register...\n");
    fflush(stdout);
    reg.primary_value ^= 0x01; // Bit flip in primary (15 -> 14)
    
    execute_dmr_operation(&reg, 10);
    assert(reg.commit_gate == CUTOFF_STATE); // Gated path (cutoff)
    assert(reg.hardware_fault_flag == true);   // Fault flagged
    printf("   ✓ DMR comparison failed. Hardware fault flagged. Write blocked.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("EDTC SELF-CHECKING TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
