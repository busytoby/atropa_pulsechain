#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Power Amplifier Level representation
typedef struct {
    double i_ref;          // Reference current (mA)
    double i_out;          // Output drive current (mA)
    double coax_z0;        // Connected coaxial line impedance (Ohms)
    gate_state_t amp_gate; // Power amplifier output isolation gate
} power_amp_t;

#define TAG_KERNEL 0x01
#define TAG_USER   0x02

// -------------------------------------------------------------
// Power Amplifier Drive & SWR Protection Execution Loop
// -------------------------------------------------------------
bool power_amp_drive(power_amp_t *amp, double target_gain, uint8_t tag) {
    // 1. SWR Protection: Cutoff output if coaxial impedance is mismatched
    // Reflected power protection triggers if coax_z0 is outside 50 Ohms +/- 2 Ohms
    if (amp->coax_z0 < 48.0 || amp->coax_z0 > 52.0) {
        amp->amp_gate = CUTOFF_STATE;
        amp->i_out = 0.0;
        return false; // Halted: Standing Wave Ratio violation
    }

    // 2. Privilege Tag Enforcement
    double active_gain = target_gain;
    if (tag != TAG_KERNEL) {
        // Restrict unprivileged amplification to unity gain (1.0x)
        active_gain = 1.0;
    }

    // 3. Current Mirror Conduction: Scale output drive current
    amp->amp_gate = CONDUC_STATE;
    amp->i_out = amp->i_ref * active_gain;
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT POWER AMPLIFIER LEVEL VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    power_amp_t amp = {
        .i_ref = 10.0, // 10 mA baseline
        .i_out = 0.0,
        .coax_z0 = 50.0, // Perfect impedance match
        .amp_gate = CUTOFF_STATE
    };

    // 1. Authorized Boost (Kernel Tag, Matched Impedance) -> Should multiply output current
    printf("[TEST] Driving power amplifier with TAG_KERNEL and matched impedance...\n");
    fflush(stdout);
    bool ok = power_amp_drive(&amp, 4.5, TAG_KERNEL); // 4.5x gain boost
    assert(ok == true);
    assert(amp.amp_gate == CONDUC_STATE);
    assert(amp.i_out == 45.0); // 10mA * 4.5
    printf("   ✓ Output current magnified successfully: %.2f mA\n", amp.i_out);
    fflush(stdout);

    // 2. Unauthorized Boost (User Tag) -> Should restrict gain to unity (1.0x)
    printf("[TEST] Driving power amplifier with TAG_USER (User clearance limit)...\n");
    fflush(stdout);
    ok = power_amp_drive(&amp, 4.5, TAG_USER);
    assert(ok == true);
    assert(amp.amp_gate == CONDUC_STATE);
    assert(amp.i_out == 10.0); // Restressed to unity
    printf("   ✓ Amplification restricted to unity gain: %.2f mA\n", amp.i_out);
    fflush(stdout);

    // 3. SWR Impedance Mismatch Protection (Coax Z0 = 75 Ohms) -> Should trigger cutoff
    printf("[TEST] Driving power amplifier with mismatched impedance (75 Ohms)...\n");
    fflush(stdout);
    amp.coax_z0 = 75.0; // Mismatch
    
    ok = power_amp_drive(&amp, 4.5, TAG_KERNEL);
    assert(ok == false); // SWR Halt
    assert(amp.amp_gate == CUTOFF_STATE); // Cutoff active
    assert(amp.i_out == 0.0); // Drive current isolated to 0 mA
    printf("   ✓ Reflected power protection isolated output current.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("POWER AMPLIFIER LEVEL TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
