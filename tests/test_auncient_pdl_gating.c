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

// PDL Program Execution Context
typedef struct {
    uint32_t state_val;
    gate_state_t write_gate;
} pdl_context_t;

// -------------------------------------------------------------
// PDL Program Operators
// -------------------------------------------------------------

// PDL Test Operator: ?phi
bool pdl_test(pdl_context_t *ctx, bool phi) {
    if (!phi) {
        ctx->write_gate = CUTOFF_STATE;
        return false; // Test failed: program execution halted
    }
    ctx->write_gate = CONDUC_STATE;
    return true; // Test passed: program continues
}

// PDL Composition: (?phi ; alpha) U (?~phi ; beta)
void pdl_execute_conditional(pdl_context_t *ctx, bool phi, 
                             void (*alpha)(pdl_context_t *), 
                             void (*beta)(pdl_context_t *)) {
    // Branchless selection based on PDL test operators
    if (pdl_test(ctx, phi)) {
        alpha(ctx);
    } else {
        // Reset gate to test negative condition
        ctx->write_gate = CONDUC_STATE; 
        if (pdl_test(ctx, !phi)) {
            beta(ctx);
        }
    }
}

// Simulated programs alpha and beta
void program_alpha(pdl_context_t *ctx) {
    ctx->state_val = 100;
}

void program_beta(pdl_context_t *ctx) {
    ctx->state_val = 200;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PDL PROGRAM GATING VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    pdl_context_t ctx = { .state_val = 0, .write_gate = CUTOFF_STATE };

    // 1. Execute conditional with phi = true -> Should execute program_alpha
    printf("[TEST] Executing PDL conditional with phi = true...\n");
    fflush(stdout);
    pdl_execute_conditional(&ctx, true, program_alpha, program_beta);
    assert(ctx.write_gate == CONDUC_STATE);
    assert(ctx.state_val == 100);
    printf("   ✓ Program alpha executed successfully: state = %u\n", ctx.state_val);
    fflush(stdout);

    // Reset context
    ctx.state_val = 0;
    ctx.write_gate = CUTOFF_STATE;

    // 2. Execute conditional with phi = false -> Should execute program_beta
    printf("[TEST] Executing PDL conditional with phi = false...\n");
    fflush(stdout);
    pdl_execute_conditional(&ctx, false, program_alpha, program_beta);
    assert(ctx.write_gate == CONDUC_STATE);
    assert(ctx.state_val == 200);
    printf("   ✓ Program beta executed successfully: state = %u\n", ctx.state_val);
    fflush(stdout);

    // 3. Direct failing test operator -> Should trigger CUTOFF_STATE immediately
    printf("[TEST] Executing failing test operator ?(false)...\n");
    fflush(stdout);
    bool test_ok = pdl_test(&ctx, false);
    assert(test_ok == false);
    assert(ctx.write_gate == CUTOFF_STATE); // Cutoff active
    printf("   ✓ Test failed: write gate cut off successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("PDL PROGRAM GATING TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
