#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
    uint32_t ball_a_pos;   // Position of Ball A (0 to 100)
    uint32_t ball_b_pos;   // Position of Ball B (0 to 100)
    uint32_t pll_phase;    // PLL clock phase in degrees (0 to 360)
    bool join_gate_active; // Join operator status
} convergence_sim_t;

// -------------------------------------------------------------
// APDL Convergence Cycle Simulator
// -------------------------------------------------------------
void simulate_apdl_cycle(convergence_sim_t *sim) {
    // 1. Advance rolling balls (Kinetic Propagation)
    if (sim->ball_a_pos < 100) {
        // Zero-crossing check at position 50 (Phase Damping)
        if (sim->ball_a_pos == 50 && (sim->pll_phase != 0 && sim->pll_phase != 180)) {
            // Held at gate: wait for phase alignment
        } else {
            sim->ball_a_pos += 10;
        }
    }

    if (sim->ball_b_pos < 100) {
        // Zero-crossing check at position 50 (Phase Damping)
        if (sim->ball_b_pos == 50 && (sim->pll_phase != 0 && sim->pll_phase != 180)) {
            // Held at gate: wait for phase alignment
        } else {
            sim->ball_b_pos += 10;
        }
    }

    // Update phase angle (advances by 90 degrees each cycle)
    sim->pll_phase = (sim->pll_phase + 90) % 360;

    // 2. Multi-Path Convergence (Join Gate)
    // Conduction path only completes when both rolling balls reach 100
    if (sim->ball_a_pos == 100 && sim->ball_b_pos == 100) {
        sim->join_gate_active = true;
    } else {
        sim->join_gate_active = false;
    }
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT APDL CONVERGENCE VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    convergence_sim_t sim = {
        .ball_a_pos = 0,
        .ball_b_pos = 0,
        .pll_phase = 90, // Out of phase initially
        .join_gate_active = false
    };

    // 1. Simulate propagation up to zero-crossing boundary
    printf("[TEST] Simulating kinetic propagation to zero-crossing...\n");
    fflush(stdout);
    for (int i = 0; i < 5; i++) {
        simulate_apdl_cycle(&sim);
    }
    assert(sim.ball_a_pos == 50);
    assert(sim.ball_b_pos == 50);
    assert(sim.pll_phase == 180); // Phase advances to zero-crossing point
    printf("   ✓ Balls reached zero-crossing boundary in-phase.\n");
    fflush(stdout);

    // 2. Advance past boundary and check for join gate convergence
    printf("[TEST] Advancing balls to destination join gate...\n");
    fflush(stdout);
    while (!sim.join_gate_active) {
        simulate_apdl_cycle(&sim);
    }
    assert(sim.ball_a_pos == 100);
    assert(sim.ball_b_pos == 100);
    assert(sim.join_gate_active == true);
    printf("   ✓ Join gate converged successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("APDL CONVERGENCE TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
