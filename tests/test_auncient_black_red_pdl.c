#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// NPN (Black) and PNP (Red) Transistor Gate Mapping
typedef struct {
    uint8_t phi_npn_black;  // Active-High NPN Gating (Precondition phi)
    uint8_t psi_pnp_red;    // Active-Low PNP Gating (Postcondition psi)
    gate_state_t write_gate;
} black_red_gate_t;

// -------------------------------------------------------------
// Black-Red PDL Transistor Gating Evaluation
// -------------------------------------------------------------
bool evaluate_black_red_gating(black_red_gate_t *gate, bool phi_val, bool psi_val) {
    gate->write_gate = CUTOFF_STATE;

    // 1. Precondition phi (NPN Black): Conducts when active-high (phi_val is true)
    gate->phi_npn_black = phi_val ? 1 : 0;

    // 2. Postcondition psi (PNP Red): Conducts when active-low (psi_val is false)
    gate->psi_pnp_red = psi_val ? 1 : 0;

    // Conduction path only open if NPN Black is high and PNP Red is low
    if (gate->phi_npn_black == 1 && gate->psi_pnp_red == 0) {
        gate->write_gate = CONDUC_STATE;
        return true; // Implication approved
    }

    return false; // Cutoff: implication failed
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BLACK-RED PDL TRANSISTOR GATING VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    black_red_gate_t gate = {
        .phi_npn_black = 0,
        .psi_pnp_red = 1,
        .write_gate = CUTOFF_STATE
    };

    // 1. Compliant state (phi = true [Black High], psi = false [Red Low]) -> Should conduct
    printf("[TEST] Evaluating gating with phi = true (Black High) and psi = false (Red Low)...\n");
    fflush(stdout);
    bool ok = evaluate_black_red_gating(&gate, true, false);
    assert(ok == true);
    assert(gate.write_gate == CONDUC_STATE);
    printf("   ✓ Gating approved. Conduction path open.\n");
    fflush(stdout);

    // 2. Precondition fail (phi = false [Black Low]) -> Should cutoff
    printf("[TEST] Evaluating gating with failed precondition phi = false...\n");
    fflush(stdout);
    ok = evaluate_black_red_gating(&gate, false, false);
    assert(ok == false);
    assert(gate.write_gate == CUTOFF_STATE);
    printf("   ✓ NPN Black cutoff verified.\n");
    fflush(stdout);

    // 3. Postcondition fail (psi = true [Red High]) -> Should cutoff
    printf("[TEST] Evaluating gating with failed postcondition psi = true...\n");
    fflush(stdout);
    ok = evaluate_black_red_gating(&gate, true, true);
    assert(ok == false);
    assert(gate.write_gate == CUTOFF_STATE);
    printf("   ✓ PNP Red cutoff verified.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("BLACK-RED PDL GATING TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
