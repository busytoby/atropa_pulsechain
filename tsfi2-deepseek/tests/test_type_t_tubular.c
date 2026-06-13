#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "tsfi_valve.h"
#include "lau_memory.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NUM_SAMPLES 256
#define SAMPLING_RATE 44100.0

int main() {
    printf("=== TSFi Type 'T' Tubular vs. Type 'S' Flat-Plate Audion Comparison ===\n");

    // Initialize two valves with identical basic parameters
    TsfiValveTriode type_s;
    TsfiValveTriode type_t;

    tsfi_valve_init(&type_s, 100.0, 0.00002, 250.0, -1.5);
    type_s.is_tubular = 0; // Standard Flat-Plate (S)

    tsfi_valve_init(&type_t, 100.0, 0.00002, 250.0, -1.5);
    type_t.is_tubular = 1; // Tubular (T) - Cylindrical coordinate electrostatic field

    float *vg_in = (float*)lau_memalign(64, NUM_SAMPLES * sizeof(float));
    float *vp_out_s = (float*)lau_memalign(64, NUM_SAMPLES * sizeof(float));
    float *vp_out_t = (float*)lau_memalign(64, NUM_SAMPLES * sizeof(float));

    // Generate 1 kHz sine wave input grid signal (amplitude 0.5 V)
    for (int i = 0; i < NUM_SAMPLES; i++) {
        vg_in[i] = 0.5f * sinf(2.0f * (float)M_PI * 1000.0f * (float)i / (float)SAMPLING_RATE);
    }

    // Process both valves with regenerative feedback beta = 0.1
    tsfi_valve_process_regenerative(&type_s, vg_in, vp_out_s, NUM_SAMPLES, 0.2, 1.0, 0.1);
    tsfi_valve_process_regenerative(&type_t, vg_in, vp_out_t, NUM_SAMPLES, 0.2, 1.0, 0.1);

    // Calculate dynamic outputs
    float min_s = vp_out_s[0], max_s = vp_out_s[0];
    float min_t = vp_out_t[0], max_t = vp_out_t[0];

    for (int i = 0; i < NUM_SAMPLES; i++) {
        if (vp_out_s[i] < min_s) min_s = vp_out_s[i];
        if (vp_out_s[i] > max_s) max_s = vp_out_s[i];

        if (vp_out_t[i] < min_t) min_t = vp_out_t[i];
        if (vp_out_t[i] > max_t) max_t = vp_out_t[i];
    }

    float p2p_s = max_s - min_s;
    float p2p_t = max_t - min_t;

    printf("[RESULTS] Peak-to-Peak Output Voltage:\n");
    printf("  Type 'S' Standard Flat-Plate:  %.4f V\n", p2p_s);
    printf("  Type 'T' Tubular (Cylindrical): %.4f V\n", p2p_t);

    // Because the Tubular Audion concentrates the field logarithmically near the axial filament (geom_scale = 0.85),
    // it yields slightly different gain dynamics compared to the flat plane model.
    assert(p2p_s != p2p_t);

    lau_free(vg_in);
    lau_free(vp_out_s);
    lau_free(vp_out_t);

    printf("[SUCCESS] Type 'T' vs Type 'S' characteristics verified!\n");
    return 0;
}
