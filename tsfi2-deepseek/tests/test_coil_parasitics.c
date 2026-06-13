#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "tsfi_lc_tank.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLING_RATE 3000000.0 // 3.0 MHz sampling rate to respect Nyquist for ~900 kHz signals
#define DURATION_SEC 0.01
#define NUM_SAMPLES (int)(SAMPLING_RATE * DURATION_SEC)

int main() {
    printf("=== TSFi2 Distributed Parameter Wavemeter Coil Demo ===\n");

    // Initialize physical coil dimensions matching historical 1916 wavemeters:
    // diameter = 0.0762 meters (3 inches)
    // height = 0.1016 meters (4 inches)
    // turns = 150
    // wire_gauge = 0.8128 mm (20 AWG)
    // C_tune = 160 pF
    double diameter = 0.0762;
    double height = 0.1016;
    double turns = 150.0;
    double wire_gauge_mm = 0.8128;
    double C_tune = 160.0e-12;

    TsfiDistributedCoil coil_ideal;
    TsfiDistributedCoil coil_parasitic;

    // Initialize 2D FDTD coils
    tsfi_distributed_coil_init(&coil_ideal, diameter, height, turns, wire_gauge_mm, C_tune);
    
    // Force ideal coil dielectric to vacuum permitivity everywhere
    double eps0 = 8.8541878128e-12;
    for (int x = 0; x < GRID_X; x++) {
        for (int y = 0; y < GRID_Y; y++) {
            for (int z = 0; z < GRID_Z; z++) {
                coil_ideal.eps[x][y][z] = eps0;
                coil_ideal.sigma[x][y][z] = 0.0;
            }
        }
    }

    tsfi_distributed_coil_init(&coil_parasitic, diameter, height, turns, wire_gauge_mm, C_tune);

    float *input_rf = (float*)malloc(NUM_SAMPLES * sizeof(float));
    float *out_ideal = (float*)malloc(NUM_SAMPLES * sizeof(float));
    float *out_parasitic = (float*)malloc(NUM_SAMPLES * sizeof(float));

    // Sweep frequency from 400 kHz to 900 kHz to capture the shift
    for (int i = 0; i < NUM_SAMPLES; i++) {
        double t = (double)i / SAMPLING_RATE;
        double freq = 400000.0 + (500000.0 / DURATION_SEC) * t;
        double phase = 2.0 * M_PI * freq * t;
        input_rf[i] = (float)sin(phase);
    }

    // Process using the 2D FDTD EM solver
    for (int i = 0; i < NUM_SAMPLES; i++) {
        double t = (double)i / SAMPLING_RATE;
        double freq = 400000.0 + (500000.0 / DURATION_SEC) * t;
        
        // Single sample step execution
        float rf_val = input_rf[i];
        float ideal_val = 0.0f;
        float par_val = 0.0f;
        
        tsfi_distributed_coil_process(&coil_ideal, &rf_val, &ideal_val, 1, SAMPLING_RATE, freq);
        tsfi_distributed_coil_process(&coil_parasitic, &rf_val, &par_val, 1, SAMPLING_RATE, freq);
        
        out_ideal[i] = ideal_val;
        out_parasitic[i] = par_val;
    }

    // Peak detector
    double max_val_ideal = 0.0;
    double max_freq_ideal = 0.0;
    double max_val_par = 0.0;
    double max_freq_par = 0.0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        double t = (double)i / SAMPLING_RATE;
        double freq = 400000.0 + (500000.0 / DURATION_SEC) * t;

        double val_ideal = fabs(out_ideal[i]);
        if (val_ideal > max_val_ideal) {
            max_val_ideal = val_ideal;
            max_freq_ideal = freq;
        }

        double val_par = fabs(out_parasitic[i]);
        if (val_par > max_val_par) {
            max_val_par = val_par;
            max_freq_par = freq;
        }
    }

    printf("[DEMO] Ideal Distributed 2D Coil Resonance: %.2f kHz\n", max_freq_ideal / 1000.0);
    printf("[DEMO] Real (Medhurst) 2D Coil Resonance: %.2f kHz\n", max_freq_par / 1000.0);

    free(input_rf);
    free(out_ideal);
    free(out_parasitic);
    return 0;
}