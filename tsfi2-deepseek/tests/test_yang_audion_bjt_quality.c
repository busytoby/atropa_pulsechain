#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "tsfi_valve.h"
#include "tsfi_fourier.h"
#include "lau_memory.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FFT_SIZE 512
#define SAMPLING_RATE 44100.0

int main() {
    printf("=== TSFi YANG Audio Synthesis Quality & Harmonic Verification ===\n");

    // 1. Setup Signal Arrays
    float *input_grid = (float*)lau_memalign(64, FFT_SIZE * sizeof(float));
    float *output_plate = (float*)lau_memalign(64, FFT_SIZE * sizeof(float));
    double *fft_real = (double*)lau_memalign(64, FFT_SIZE * sizeof(double));
    double *fft_imag = (double*)lau_memalign(64, FFT_SIZE * sizeof(double));
    double *magnitude = (double*)lau_memalign(64, (FFT_SIZE / 2) * sizeof(double));

    // Generate a pure 1 kHz fundamental sine wave grid input (8 cycles over 512 samples)
    float freq_fundamental = 1000.0f;
    for (int i = 0; i < FFT_SIZE; i++) {
        input_grid[i] = 1.0f * sinf(2.0f * (float)M_PI * freq_fundamental * (float)i / (float)SAMPLING_RATE);
    }

    // Initialize DeForest Soft-Vacuum Valve
    TsfiValveTriode valve;
    tsfi_valve_init(&valve, 100.0, 0.00002, 250.0, -1.5); // mu=100, K=0.00002, Vp=250V, Vg_bias=-1.5V

    // Process through the Soft-Vacuum AVX-512 physics engine (No panic: eta=0.0, kappa=1.0)
    tsfi_valve_process_avx512(&valve, input_grid, output_plate, FFT_SIZE, 0.0, 1.0);

    // Determine the idle value (DC offset)
    float idle_vg[16] = {0};
    float idle_vp[16] = {0};
    tsfi_valve_process_avx512(&valve, idle_vg, idle_vp, 16, 0.0, 1.0);
    float dc_offset = idle_vp[0];

    // 2. Perform Fourier Analysis on the AC output component
    for (int i = 0; i < FFT_SIZE; i++) {
        fft_real[i] = (double)(output_plate[i] - dc_offset);
        fft_imag[i] = 0.0;
    }

    // Standard raw discrete Fourier transform
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

    // 3. Extract Harmonic Peaks
    // Bin indices: bin = freq * FFT_SIZE / SAMPLING_RATE
    // For 1000 Hz: bin = 1000 * 512 / 44100 = 11.6 (bin 12)
    // For 2000 Hz: bin = 2000 * 512 / 44100 = 23.2 (bin 23)
    // For 3000 Hz: bin = 3000 * 512 / 44100 = 34.8 (bin 35)
    double mag_fundamental = magnitude[12];
    double mag_2nd_harmonic = magnitude[23];
    double mag_3rd_harmonic = magnitude[35];

    printf("[ANALYSIS] DC Offset (Idle Vp): %.2f V\n", dc_offset);
    printf("[ANALYSIS] Spectral Magnitudes:\n");
    printf("  1 kHz Fundamental: %.6f\n", mag_fundamental);
    printf("  2 kHz (Even, 2nd Harmonic): %.6f\n", mag_2nd_harmonic);
    printf("  3 kHz (Odd, 3rd Harmonic):  %.6f\n", mag_3rd_harmonic);

    // Prove Valve Duality: Soft-vacuum Child's law generates predominantly even-order harmonics (2nd > 3rd)
    printf("[PROOF] Even-to-Odd Harmonic Ratio (2nd / 3rd): %.2f\n", mag_2nd_harmonic / mag_3rd_harmonic);
    assert(mag_2nd_harmonic > mag_3rd_harmonic);

    // Cleanup
    lau_free(input_grid);
    lau_free(output_plate);
    lau_free(fft_real);
    lau_free(fft_imag);
    lau_free(magnitude);

    printf("[SUCCESS] YANG Audio quality verified! Valve-harmonic distribution satisfies physical proofs.\n");
    return 0;
}
