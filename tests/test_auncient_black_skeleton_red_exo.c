#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Black SKELETON (NPN) & Red EXOSKELETON (PNP) Gating State Map
typedef struct {
    uint8_t skeleton_heartbeat_high; // NPN Black: Active-High Guest Heartbeat (1 = Active)
    uint8_t exoskeleton_error_low;  // PNP Red: Active-Low Supervisor Error Flag (0 = No Error)
    gate_state_t write_gate;         // Global write conduction gate
} black_skeleton_red_exo_t;

// -------------------------------------------------------------
// Gating Path Evaluation
// -------------------------------------------------------------
bool evaluate_skeleton_exoskeleton_gating(black_skeleton_red_exo_t *gate) {
    gate->write_gate = CUTOFF_STATE;

    // Conduction path is open only when:
    // 1. SKELETON (NPN Black) is high (1)
    // 2. EXOSKELETON (PNP Red) is low (0)
    if (gate->skeleton_heartbeat_high == 1 && gate->exoskeleton_error_low == 0) {
        gate->write_gate = CONDUC_STATE;
        return true; // Path conducts
    }

    return false; // Cutoff active
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BLACK SKELETON RED EXOSKELETON GATING VALIDATION\n");
    printf("=============================================================\n");
    fflush(stdout);

    black_skeleton_red_exo_t gate = {
        .skeleton_heartbeat_high = 0,
        .exoskeleton_error_low = 1,
        .write_gate = CUTOFF_STATE
    };

    // 1. Compliant state: Guest active (1), no error (0) -> Should conduct
    printf("[TEST] Evaluating: Guest active (1), no error (0)...\n");
    fflush(stdout);
    gate.skeleton_heartbeat_high = 1;
    gate.exoskeleton_error_low = 0;
    bool ok = evaluate_skeleton_exoskeleton_gating(&gate);
    assert(ok == true);
    assert(gate.write_gate == CONDUC_STATE);
    printf("   ✓ Conduction path open. SKELETON and EXOSKELETON aligned.\n");
    fflush(stdout);

    // 2. Guest hang: Heartbeat low (0), no error (0) -> Should cutoff
    printf("[TEST] Evaluating guest hang: Heartbeat low (0)...\n");
    fflush(stdout);
    gate.skeleton_heartbeat_high = 0;
    gate.exoskeleton_error_low = 0;
    ok = evaluate_skeleton_exoskeleton_gating(&gate);
    assert(ok == false);
    assert(gate.write_gate == CUTOFF_STATE);
    printf("   ✓ NPN Black gate cutoff verified on guest hang.\n");
    fflush(stdout);

    // 3. Supervisor error: Guest active (1), error high (1) -> Should cutoff
    printf("[TEST] Evaluating supervisor trap: Error high (1)...\n");
    fflush(stdout);
    gate.skeleton_heartbeat_high = 1;
    gate.exoskeleton_error_low = 1;
    ok = evaluate_skeleton_exoskeleton_gating(&gate);
    assert(ok == false);
    assert(gate.write_gate == CUTOFF_STATE);
    printf("   ✓ PNP Red gate cutoff verified on supervisor error.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("BLACK SKELETON RED EXOSKELETON GATING TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
