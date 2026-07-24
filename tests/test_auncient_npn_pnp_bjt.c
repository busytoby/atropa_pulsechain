#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#define SIGNAL_LENGTH 64

// Transistor conduction states
typedef enum {
    TRANSISTOR_CUTOFF,  // OFF state
    TRANSISTOR_ACTIVE   // ON state
} transistor_state_t;

// NPN Black (Even / Cosine) Transistor structure
typedef struct {
    transistor_state_t state;
    double base_current;
    double collector_voltage;
} npn_black_t;

// PNP Red (Odd / Sine) Transistor structure
typedef struct {
    transistor_state_t state;
    double base_current;
    double emitter_voltage;
} pnp_red_t;

// -------------------------------------------------------------
// BJT Switching Emulation Gated by QFT Segregates
// -------------------------------------------------------------
void update_npn_black_state(npn_black_t *npn, double even_amplitude) {
    // NPN Black is active-high; conduction occurs when positive carrier base threshold is met
    npn->base_current = even_amplitude > 0.0 ? even_amplitude * 0.1 : 0.0;
    
    if (npn->base_current > 0.5) {
        npn->state = TRANSISTOR_ACTIVE;
        npn->collector_voltage = 0.2; // Saturated voltage drop
    } else {
        npn->state = TRANSISTOR_CUTOFF;
        npn->collector_voltage = 5.0; // Open circuit voltage
    }
}

void update_pnp_red_state(pnp_red_t *pnp, double odd_amplitude) {
    // PNP Red is active-low; conduction occurs when base current flows out (negative amplitude)
    pnp->base_current = odd_amplitude < 0.0 ? -odd_amplitude * 0.1 : 0.0;
    
    if (pnp->base_current > 0.5) {
        pnp->state = TRANSISTOR_ACTIVE;
        pnp->emitter_voltage = 4.8; // Active conduction voltage
    } else {
        pnp->state = TRANSISTOR_CUTOFF;
        pnp->emitter_voltage = 0.0; // Cutoff voltage
    }
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT NPN BLACK & PNP RED TRANSISTOR EMULATION PIPELINE\n");
    printf("=============================================================\n");
    fflush(stdout);

    npn_black_t npn_black = { TRANSISTOR_CUTOFF, 0.0, 5.0 };
    pnp_red_t pnp_red = { TRANSISTOR_CUTOFF, 0.0, 0.0 };

    // 1. Test NPN Black (Symmetric Cosine) Activation
    printf("[TEST] Verifying NPN Black (Even Component) switching states...\n");
    fflush(stdout);
    
    // Low amplitude -> Cutoff
    update_npn_black_state(&npn_black, 2.0);
    assert(npn_black.state == TRANSISTOR_CUTOFF);
    assert(npn_black.collector_voltage == 5.0);

    // High amplitude -> Active conduction
    update_npn_black_state(&npn_black, 8.0);
    assert(npn_black.state == TRANSISTOR_ACTIVE);
    assert(npn_black.collector_voltage == 0.2);
    printf("   ✓ NPN Black switching states verified.\n");
    fflush(stdout);

    // 2. Test PNP Red (Antisymmetric Sine) Activation
    printf("[TEST] Verifying PNP Red (Odd Component) switching states...\n");
    fflush(stdout);
    
    // Positive amplitude (No base current draw) -> Cutoff
    update_pnp_red_state(&pnp_red, 4.0);
    assert(pnp_red.state == TRANSISTOR_CUTOFF);
    assert(pnp_red.emitter_voltage == 0.0);

    // Negative amplitude (Base current drawn out) -> Active conduction
    update_pnp_red_state(&pnp_red, -8.0);
    assert(pnp_red.state == TRANSISTOR_ACTIVE);
    assert(pnp_red.emitter_voltage == 4.8);
    printf("   ✓ PNP Red switching states verified.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("NPN BLACK & PNP RED EMULATION PIPELINE VERIFIED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
