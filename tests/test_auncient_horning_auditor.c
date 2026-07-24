#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// XPL Variable representation
typedef struct {
    char name[32];
    uint32_t memory_address;
} xpl_var_t;

// -------------------------------------------------------------
// Horning-Style Compile-Time Aliasing Auditor
// -------------------------------------------------------------
bool tws_verify_aliasing(const xpl_var_t *vars, size_t count, gate_state_t *comp_gate) {
    *comp_gate = CONDUC_STATE;

    // Scan for aliasing: two different names referencing the same memory address
    for (size_t i = 0; i < count; i++) {
        for (size_t j = i + 1; j < count; j++) {
            if (vars[i].memory_address == vars[j].memory_address) {
                // Aliasing detected -> force compiler cutoff to prevent side effects
                *comp_gate = CUTOFF_STATE;
                return false;
            }
        }
    }

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT HORNING ALIASING AUDITOR VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    gate_state_t comp_gate = CUTOFF_STATE;

    // 1. Compliant variable layout (No overlapping memory addresses)
    xpl_var_t vars_ok[] = {
        { "reg_accum", 0xF300 },
        { "reg_status", 0xF304 },
        { "reg_buffer", 0xF400 }
    };
    
    printf("[TEST] Analyzing compliant variable table (No aliasing)...\n");
    fflush(stdout);
    bool verification_ok = tws_verify_aliasing(vars_ok, 3, &comp_gate);
    assert(verification_ok == true);
    assert(comp_gate == CONDUC_STATE);
    printf("   ✓ Verification passed: Compilation gate conducts successfully.\n");
    fflush(stdout);

    // 2. Non-compliant variable layout (Aliased memory addresses)
    xpl_var_t vars_aliased[] = {
        { "reg_accum", 0xF300 },
        { "reg_alias", 0xF300 }, // Overlaps with reg_accum!
        { "reg_buffer", 0xF400 }
    };

    printf("[TEST] Analyzing non-compliant variable table (Aliasing present)...\n");
    fflush(stdout);
    verification_ok = tws_verify_aliasing(vars_aliased, 3, &comp_gate);
    assert(verification_ok == false);
    assert(comp_gate == CUTOFF_STATE); // Cutoff active, compilation blocked
    printf("   ✓ Aliasing detected: compilation blocked at boundary.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("HORNING ALIASING AUDITOR TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
