#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Simulated CSCW-Black and CRDT-Red Gating layout
typedef struct {
    uint32_t cscw_black_npn; // Active-high NPN Gate (5V = active, 0V = cutoff)
    uint32_t crdt_red_pnp;   // Active-low PNP Gate (0V = active, 5V = cutoff)
    gate_state_t conduction_path;
} cscw_crdt_gate_t;

// -------------------------------------------------------------
// Gating Logic (CSCW is black, CRDT is red)
// -------------------------------------------------------------
void evaluate_cscw_crdt_gate(cscw_crdt_gate_t *gate) {
    // Conduction occurs only if:
    // CSCW Black NPN gate is High (active-high) AND
    // CRDT Red PNP gate is Low (active-low)
    if (gate->cscw_black_npn == 1 && gate->crdt_red_pnp == 0) {
        gate->conduction_path = CONDUC_STATE;
    } else {
        gate->conduction_path = CUTOFF_STATE;
    }
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT CSCW-BLACK CRDT-RED TRANSISTOR GATING SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    cscw_crdt_gate_t gate = {
        .cscw_black_npn = 0,
        .crdt_red_pnp = 1,
        .conduction_path = CUTOFF_STATE
    };

    // 1. Cutoff Case: Both gates inactive -> Should remain cutoff
    evaluate_cscw_crdt_gate(&gate);
    assert(gate.conduction_path == CUTOFF_STATE);
    printf("[TEST] Inactive state verified (cutoff).\n");
    fflush(stdout);

    // 2. Conduction Case: CSCW Black NPN is 1, CRDT Red PNP is 0 -> Should conduct
    gate.cscw_black_npn = 1;
    gate.crdt_red_pnp = 0;
    evaluate_cscw_crdt_gate(&gate);
    assert(gate.conduction_path == CONDUC_STATE);
    printf("[TEST] Conduction state verified (CSCW = 1, CRDT = 0 -> open path).\n");
    fflush(stdout);

    // 3. Cutoff Case: CRDT Red PNP is high -> Should cutoff
    gate.cscw_black_npn = 1;
    gate.crdt_red_pnp = 1;
    evaluate_cscw_crdt_gate(&gate);
    assert(gate.conduction_path == CUTOFF_STATE);
    printf("[TEST] Red PNP high cutoff verified.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("CSCW-BLACK CRDT-RED TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
