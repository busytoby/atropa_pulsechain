#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "tsfi_valve.h"

// --- Thermionic Valve (Triode) Synthesis Test ---
// Feed a sine wave to the grid and verify that the output exhibits
// Child's Law non-linear characteristics (dominance of second harmonic).

#define NUM_SAMPLES 256
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main() {
    printf("=== TSFi2 Thermionic Valve Synthesis Test ===\n");

    // Initialize triode:
    // - Amplification factor mu = 100.0 (12AX7 typical)
    // - Perveance K = 0.00002
    // - Plate supply Vp = 250.0V
    // - Grid bias Vg_bias = -1.5V
    TsfiValveTriode valve;
    tsfi_valve_init(&valve, 100.0, 0.00002, 250.0, -1.5);

    // Generate input sine wave (Grid voltage variation: peak-to-peak 2.0V)
    float *vg_in = malloc(NUM_SAMPLES * sizeof(float));
    float *vp_out = malloc(NUM_SAMPLES * sizeof(float));
    assert(vg_in && vp_out);

    for (int i = 0; i < NUM_SAMPLES; i++) {
        vg_in[i] = 1.0f * (float)sin(2.0 * M_PI * i / 64.0); // 4 full cycles
    }

    // Process using YI reaction parameters:
    // - Ichidai (eta) = 0.0 (nominal mu)
    // - Daiichi (kappa) = 1.0 (nominal K)
    printf("[VALVE] Processing %d samples through Child's Law triode...\n", NUM_SAMPLES);
    tsfi_valve_process_avx512(&valve, vg_in, vp_out, NUM_SAMPLES, 0.0, 1.0);

    // Verify non-linear asymmetry (presence of second harmonic)
    // We check if the positive half-cycles and negative half-cycles of the plate output
    // have asymmetric deviations from the nominal DC bias.
    // At idle (Vg_in = 0), Vp should be some DC value:
    float idle_vg[16] = {0};
    float idle_vp[16] = {0};
    tsfi_valve_process_avx512(&valve, idle_vg, idle_vp, 16, 0.0, 1.0);
    float idle_val = idle_vp[0];
    printf("  [PHYSICS] Idle Plate Voltage: %.2f V\n", idle_val);

    float max_dev_pos = 0.0f;
    float max_dev_neg = 0.0f;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        float dev = vp_out[i] - idle_val;
        if (dev > max_dev_pos) max_dev_pos = dev;
        if (dev < max_dev_neg) max_dev_neg = dev;
    }

    float abs_max_dev_pos = fabs(max_dev_pos);
    float abs_max_dev_neg = fabs(max_dev_neg);

    printf("  [ANALYSIS] Positive Peak Deviation: +%.2f V\n", abs_max_dev_pos);
    printf("  [ANALYSIS] Negative Peak Deviation: -%.2f V\n", abs_max_dev_neg);

    // In a symmetric linear system, positive and negative deviations would be equal.
    // In a space-charge triode, positive grid swing draws more plate current (soft compression)
    // causing asymmetric voltage swing at the plate.
    float asymmetry = fabs(abs_max_dev_pos - abs_max_dev_neg);
    printf("  [ANALYSIS] Harmonic Asymmetry Delta: %.4f V\n", asymmetry);

    // Assert that we have non-trivial asymmetry indicating even harmonics (warm tube distortion)
    assert(asymmetry > 1.0f);

    printf("[SUCCESS] Thermionic Valve synthesis successfully verified!\n");

    free(vg_in);
    free(vp_out);
    return 0;
}
