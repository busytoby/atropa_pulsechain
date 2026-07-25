#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define NUM_HARMONICS 8
#define DECAY_THRESHOLD 0.15

typedef struct {
    // Electrostatic grid charge values (representing dynamic drawbar harmonic levels)
    double charge_level[NUM_HARMONICS];
    double leak_rate;      // Rate of charge loss per second
    double regen_current;  // Feed-forward regeneration current replenishment rate
} electrostatic_grid_t;

// Update electrostatic grid state simulating charge leakage and regeneration loops
void tick_electrostatic_grid(electrostatic_grid_t *grid, bool regen_active, double dt) {
    for (int i = 0; i < NUM_HARMONICS; i++) {
        // 1. Natural charge leakage decay
        grid->charge_level[i] -= grid->leak_rate * dt;
        if (grid->charge_level[i] < 0.0) {
            grid->charge_level[i] = 0.0;
        }

        // 2. Active regeneration loop replenishment
        if (regen_active) {
            grid->charge_level[i] += grid->regen_current * dt;
            if (grid->charge_level[i] > 1.0) {
                grid->charge_level[i] = 1.0;
            }
        }
    }
}

// Generate the output harmonic mix scaled by active grid charge levels
double mix_electrostatic_drawbars(const electrostatic_grid_t *grid, double time) {
    double mixed_signal = 0.0;
    double base_freq = 440.0; // A4
    double ratios[NUM_HARMONICS] = { 0.5, 1.0, 1.5, 2.0, 3.0, 4.0, 5.0, 6.0 };

    for (int i = 0; i < NUM_HARMONICS; i++) {
        double active_charge = grid->charge_level[i];
        if (active_charge <= 0.0) continue;

        // Introduce grid noise grit when charge decays close to depletion limits
        double grid_noise = 0.0;
        if (active_charge < DECAY_THRESHOLD) {
            grid_noise = (((double)rand() / (double)RAND_MAX) - 0.5) * 0.25;
        }

        double phase = 2.0 * 3.14159265358979323846 * base_freq * ratios[i] * time;
        mixed_signal += sin(phase) * (active_charge + grid_noise);
    }

    return mixed_signal;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ELECTROSTATIC TUBE SUSTAIN SUITE\n");
    printf("=============================================================\n");

    electrostatic_grid_t grid = {
        .charge_level = { 1.0, 0.8, 0.6, 0.0, 0.0, 0.0, 0.0, 0.0 }, // Initialize active drawbars
        .leak_rate = 0.5,     // 50% charge decay per second
        .regen_current = 2.0  // Fast 200% replenishment rate
    };

    double dt = 0.1; // 100ms ticks

    // 1. Verify Charge decay when regeneration loop is disabled
    printf("[TEST] Testing natural leakage decay (Regen = OFF)...\n");
    tick_electrostatic_grid(&grid, false, dt);
    
    // Initial was 1.0 -> should decay to 1.0 - (0.5 * 0.1) = 0.95
    assert(grid.charge_level[0] < 1.0);
    assert(grid.charge_level[0] == 0.95);
    printf("   ✓ Natural decay confirmed. Level: %.4f\n", grid.charge_level[0]);

    // 2. Verify Regeneration Loop replenishes and stabilizes charge
    printf("[TEST] Testing regeneration loop replenishment (Regen = ON)...\n");
    tick_electrostatic_grid(&grid, true, dt);
    
    // Decay: 0.95 - (0.5 * 0.1) = 0.90 -> Regen: 0.90 + (2.0 * 0.1) = 1.10 -> clamped to 1.0
    assert(grid.charge_level[0] == 1.0);
    printf("   ✓ Regeneration replenishment confirmed. Level: %.4f\n", grid.charge_level[0]);

    // 3. Verify Grit generation at low charge levels
    printf("[TEST] Verifying signal grid noise at decay boundary...\n");
    grid.charge_level[0] = 0.08; // Below decay threshold (0.15)
    double time = 0.01;
    double out1 = mix_electrostatic_drawbars(&grid, time);
    double out2 = mix_electrostatic_drawbars(&grid, time);
    
    // Output should include random noise and differ between successive reads
    assert(out1 != out2);
    printf("   ✓ Electrostatic boundary grid noise successfully verified.\n");

    printf("=============================================================\n");
    printf("ELECTROSTATIC SUSTAIN TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
