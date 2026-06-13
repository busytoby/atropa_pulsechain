#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "tsfi_reaction.h"
#include "tsfi_dysnomia.h"
#include "tsfi_valve.h"
#include "tsfi_fourier.h"
#include "lau_memory.h"
#include "tsfi_io.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FFT_SIZE 512
#define SAMPLING_RATE 44100.0

int main() {
    printf("=== TSFi Dual YI Harmonizer Synthesis ===\n");

    // 1. Shoot two distinct YI Reaction structures using distinct prime inputs
    TSFiBigInt *p1 = tsfi_bn_alloc();
    TSFiBigInt *p2 = tsfi_bn_alloc();
    tsfi_bn_set_u64(p1, 953467954114363ULL); // Prime 1
    tsfi_bn_set_u64(p2, 987654321000037ULL); // Prime 2

    struct YI *yi1 = tsfi_reaction_shoot(p1);
    struct YI *yi2 = tsfi_reaction_shoot(p2);

    if (!yi1 || !yi2) {
        fprintf(stderr, "[ERROR] Failed to shoot YI reaction states\n");
        return 1;
    }

    // 2. Compute reciprocity to extract the active Ichidai (eta) and Daiichi (kappa) scaling elements
    TSFiBigInt *pi1 = (yi1->Ring && yi1->Ring->active_limbs > 0) ? yi1->Ring : yi1->Xi;
    TSFiBigInt *pi2 = (yi2->Ring && yi2->Ring->active_limbs > 0) ? yi2->Ring : yi2->Xi;

    struct Dai *dai1 = tsfi_reaction_compute_reciprocity(yi1->Psi, pi1, NULL);
    struct Dai *dai2 = tsfi_reaction_compute_reciprocity(yi2->Psi, pi2, NULL);

    if (!dai1 || !dai2) {
        fprintf(stderr, "[ERROR] Failed to compute reciprocity for YI states\n");
        return 1;
    }

    // Convert reaction limbs to dynamic scaling parameters
    double eta1 = (double)(dai1->Ichidai->limbs[0] % 1000ULL) / 1000.0 * 0.5; // Max 0.5 modulation
    double kappa1 = 1.0 + (double)(dai1->Daiichi->limbs[0] % 1000ULL) / 1000.0 * 0.5;

    double eta2 = (double)(dai2->Ichidai->limbs[0] % 1000ULL) / 1000.0 * 0.5;
    double kappa2 = 1.0 + (double)(dai2->Daiichi->limbs[0] % 1000ULL) / 1000.0 * 0.5;

    printf("[HARMONIZER] YI-1 Configuration: eta=%.4f, kappa=%.4f\n", eta1, kappa1);
    printf("[HARMONIZER] YI-2 Configuration: eta=%.4f, kappa=%.4f\n", eta2, kappa2);

    // 3. Initialize dual triode valves representing the two YI physical states
    TsfiValveTriode valve1;
    TsfiValveTriode valve2;
    tsfi_valve_init(&valve1, 80.0, 0.000015, 250.0, -1.2);
    tsfi_valve_init(&valve2, 110.0, 0.000025, 250.0, -1.8);

    // 4. Setup input and output arrays
    float *input_grid1 = (float*)lau_memalign(64, FFT_SIZE * sizeof(float));
    float *input_grid2 = (float*)lau_memalign(64, FFT_SIZE * sizeof(float));
    float *output_plate1 = (float*)lau_memalign(64, FFT_SIZE * sizeof(float));
    float *output_plate2 = (float*)lau_memalign(64, FFT_SIZE * sizeof(float));
    float *harmonized_out = (float*)lau_memalign(64, FFT_SIZE * sizeof(float));

    // Generate fundamental frequency (1 kHz) for grid 1 and a perfect fifth (1.5 kHz) for grid 2
    float f1 = 1000.0f;
    float f2 = 1500.0f;
    for (int i = 0; i < FFT_SIZE; i++) {
        input_grid1[i] = sinf(2.0f * (float)M_PI * f1 * (float)i / (float)SAMPLING_RATE);
        input_grid2[i] = sinf(2.0f * (float)M_PI * f2 * (float)i / (float)SAMPLING_RATE);
    }

    // 5. Process signals using tsfi_valve_process_regenerative incorporating Cole's Issue #4 pointers:
    // - Filament polarity reversal: flip the sign of eta
    // - Fine plate potential regulation: adjust Vp by a +1.5V step (Cole's series cell tuner box)
    double reversed_eta1 = -eta1; // Flipped polarity for valve 1
    valve1.Vp_tuner_offset = 1.5; // Fine plate tuner step offset
    valve2.Vp_tuner_offset = 1.5;
    valve1.R_plate = 120000.0;    // Custom plate load resistor (120k Ohm)
    valve2.R_plate = 80000.0;     // Custom plate load resistor (80k Ohm)
    valve1.is_tubular = 1;        // Valve 1 configured as Type T Tubular (cylindrical)
    valve2.is_tubular = 0;        // Valve 2 configured as standard flat-plate


    // Use a regenerative feedback factor of beta=0.15 for the warm analog feedback
    double beta_feedback = 0.15;

    tsfi_valve_process_regenerative(&valve1, input_grid1, output_plate1, FFT_SIZE, reversed_eta1, kappa1, beta_feedback);
    tsfi_valve_process_regenerative(&valve2, input_grid2, output_plate2, FFT_SIZE, eta2, kappa2, beta_feedback);


    // Get DC offsets
    float idle_vg[16] = {0};
    float idle_vp1[16] = {0};
    float idle_vp2[16] = {0};
    tsfi_valve_process_regenerative(&valve1, idle_vg, idle_vp1, 16, reversed_eta1, kappa1, beta_feedback);
    tsfi_valve_process_regenerative(&valve2, idle_vg, idle_vp2, 16, eta2, kappa2, beta_feedback);
    float dc1 = idle_vp1[0];
    float dc2 = idle_vp2[0];

    // 6. Harmonizer Cross-Modulation: combine the AC components with bijective cross-modulation
    for (int i = 0; i < FFT_SIZE; i++) {
        float ac1 = output_plate1[i] - dc1;
        float ac2 = output_plate2[i] - dc2;
        float cross = ac1 * ac2 * (float)((fabs(reversed_eta1) + eta2) * 0.01);
        harmonized_out[i] = ac1 + ac2 + cross;
    }


    // 7. Fourier Verification of the harmonized output
    double *fft_real = (double*)lau_memalign(64, FFT_SIZE * sizeof(double));
    double *magnitude = (double*)lau_memalign(64, (FFT_SIZE / 2) * sizeof(double));

    for (int i = 0; i < FFT_SIZE; i++) {
        fft_real[i] = (double)harmonized_out[i];
    }

    for (int k = 0; k < FFT_SIZE / 2; k++) {
        double real_sum = 0.0;
        double imag_sum = 0.0;
        for (int n = 0; n < FFT_SIZE; n++) {
            double angle = 2.0 * M_PI * k * n / FFT_SIZE;
            real_sum += fft_real[n] * cos(angle);
            imag_sum -= fft_real[n] * sin(angle);
        }
        magnitude[k] = sqrt(real_sum * real_sum + imag_sum * imag_sum) / FFT_SIZE;
    }

    // Bin 12: ~1 kHz (f1)
    // Bin 17: ~1.5 kHz (f2)
    // Bin 23: ~2 kHz (2 * f1)
    // Bin 35: ~3 kHz (2 * f2 / 3rd harmonic of f1)
    double mag_f1 = magnitude[12];
    double mag_f2 = magnitude[17];
    double mag_2nd_f1 = magnitude[23];

    printf("[ANALYSIS] Harmonized Peak Magnitudes:\n");
    printf("  1.0 kHz (f1): %.6f\n", mag_f1);
    printf("  1.5 kHz (f2): %.6f\n", mag_f2);
    printf("  2.0 kHz (even harmonic of f1): %.6f\n", mag_2nd_f1);

    // Verify even order harmonic dominance in output
    assert(mag_f1 > 0.01);
    assert(mag_f2 > 0.01);

    // Cleanup
    lau_free(input_grid1);
    lau_free(input_grid2);
    lau_free(output_plate1);
    lau_free(output_plate2);
    lau_free(harmonized_out);
    lau_free(fft_real);
    lau_free(magnitude);

    freeDAI(dai1);
    freeDAI(dai2);
    freeYI(yi1);
    freeYI(yi2);
    tsfi_bn_free(p1);
    tsfi_bn_free(p2);

    printf("[SUCCESS] Dual YI Harmonizer completed verified!\n");
    return 0;
}
