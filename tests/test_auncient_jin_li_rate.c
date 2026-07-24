#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Simulated Frame Rate Gating state
typedef struct {
    uint32_t last_frame_timestamp_ms;
    uint32_t min_frame_delta_ms; // Expected min delay (e.g. 16ms for ~60fps)
    gate_state_t rate_gate;
} jin_li_rate_gate_t;

// -------------------------------------------------------------
// Jin Li Rate Control Gating
// -------------------------------------------------------------
bool process_jin_li_frame_rate(jin_li_rate_gate_t *gate, uint32_t current_timestamp_ms) {
    gate->rate_gate = CUTOFF_STATE;

    // Calculate arrival delta time
    uint32_t delta = current_timestamp_ms - gate->last_frame_timestamp_ms;

    // Throttle check: If frames arrive too fast (delta < min_delta), block write to prevent buffer overflow
    if (delta < gate->min_frame_delta_ms) {
        return false; // Throttled. Cutoff active.
    }

    // Pass check: update timestamp and open conduction path
    gate->last_frame_timestamp_ms = current_timestamp_ms;
    gate->rate_gate = CONDUC_STATE;
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT JIN LI RATE CONTROL GATING VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    jin_li_rate_gate_t gate = {
        .last_frame_timestamp_ms = 1000,
        .min_frame_delta_ms = 16, // Min 16ms between frames
        .rate_gate = CUTOFF_STATE
    };

    // 1. Compliant Case: Frame arrives after 20ms delta -> Should pass
    printf("[TEST] Checking compliant frame timing (20ms delta)...\n");
    fflush(stdout);
    bool ok = process_jin_li_frame_rate(&gate, 1020);
    assert(ok == true);
    assert(gate.rate_gate == CONDUC_STATE);
    printf("   ✓ Frame committed. Conduction path open.\n");
    fflush(stdout);

    // 2. Throttled Case: Frame arrives too fast (5ms delta) -> Should fail
    printf("[TEST] Checking throttled frame timing (5ms delta)...\n");
    fflush(stdout);
    ok = process_jin_li_frame_rate(&gate, 1025); // 1025 - 1020 = 5ms (less than 16ms)
    assert(ok == false);
    assert(gate.rate_gate == CUTOFF_STATE);
    printf("   ✓ Frame throttled successfully. Rate-limiting cutoff active.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("JIN LI RATE CONTROL TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
