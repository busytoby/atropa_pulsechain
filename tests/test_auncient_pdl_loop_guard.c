#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// VDM Context
typedef struct {
    uint32_t value;
    gate_state_t write_gate;
} pdl_loop_ctx_t;

// -------------------------------------------------------------
// PDL Test Operator: ?phi
// -------------------------------------------------------------
bool pdl_test(pdl_loop_ctx_t *ctx, bool phi) {
    if (!phi) {
        ctx->write_gate = CUTOFF_STATE;
        return false;
    }
    ctx->write_gate = CONDUC_STATE;
    return true;
}

// -------------------------------------------------------------
// PDL Loop-Guard Execution: (?phi ; alpha)* ; ?phi
// -------------------------------------------------------------
bool pdl_execute_loop_guard(pdl_loop_ctx_t *ctx, bool (*invariant)(const pdl_loop_ctx_t *), 
                            void (*alpha)(pdl_loop_ctx_t *), uint32_t steps) {
    // 1. Entry Check: ?phi
    if (!pdl_test(ctx, invariant(ctx))) {
        return false; // Halted on entry
    }

    // 2. Loop Iteration: ( ?phi ; alpha )*
    for (uint32_t i = 0; i < steps; i++) {
        alpha(ctx);

        // Verify invariant after iteration step
        if (!pdl_test(ctx, invariant(ctx))) {
            return false; // Halted during iteration (invariant broken)
        }
    }

    // 3. Exit Check: ?phi
    return pdl_test(ctx, invariant(ctx));
}

// Invariant definition: value must remain under 100
bool invariant_value_limit(const pdl_loop_ctx_t *ctx) {
    return ctx->value < 100;
}

// Safe loop step (increments value by 10)
void step_safe(pdl_loop_ctx_t *ctx) {
    ctx->value += 10;
}

// Unsafe loop step (triggers overflow to 150)
void step_unsafe(pdl_loop_ctx_t *ctx) {
    ctx->value = 150;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PDL LOOP-GUARD VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    pdl_loop_ctx_t ctx = { .value = 0, .write_gate = CUTOFF_STATE };

    // 1. Safe loop execution (5 steps of +10 -> value = 50 < 100) -> Should succeed
    printf("[TEST] Running safe PDL loop guard (5 steps)...\n");
    fflush(stdout);
    bool ok = pdl_execute_loop_guard(&ctx, invariant_value_limit, step_safe, 5);
    assert(ok == true);
    assert(ctx.write_gate == CONDUC_STATE);
    assert(ctx.value == 50);
    printf("   ✓ Loop completed successfully. Invariant preserved.\n");
    fflush(stdout);

    // 2. Unsafe loop execution -> Should fail and cutoff during iteration
    printf("[TEST] Running unsafe PDL loop guard (step triggers overflow)...\n");
    fflush(stdout);
    ok = pdl_execute_loop_guard(&ctx, invariant_value_limit, step_unsafe, 5);
    assert(ok == false);
    assert(ctx.write_gate == CUTOFF_STATE); // Cutoff active
    printf("   ✓ Invariant violation caught. Loop execution halted.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("PDL LOOP-GUARD TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
