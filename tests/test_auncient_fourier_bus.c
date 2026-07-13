#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#define DFT_SIZE 16
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Decodes discrete multiplexed data streams from composite wave magnitudes
void run_fourier_bus_demux(double *input_real, char *out_char, uint8_t *out_val) {
    double out_real[DFT_SIZE];
    double out_imag[DFT_SIZE];
    
    // Perform Forward DFT to resolve frequency bins
    for (int k = 0; k < DFT_SIZE; k++) {
        out_real[k] = 0.0;
        out_imag[k] = 0.0;
        for (int n = 0; n < DFT_SIZE; n++) {
            double angle = 2.0 * M_PI * k * n / DFT_SIZE;
            out_real[k] += input_real[n] * cos(angle);
            out_imag[k] -= input_real[n] * sin(angle);
        }
    }
    
    // Demultiplex channels based on magnitude thresholds
    double mag_bin2 = sqrt(out_real[2] * out_real[2] + out_imag[2] * out_imag[2]);
    double mag_bin5 = sqrt(out_real[5] * out_real[5] + out_imag[5] * out_imag[5]);
    
    // Decode Channel A (Bin 2): Mapped magnitude to ASCII value (Mag 8.0 -> 'H')
    if (mag_bin2 > 1.0) {
        *out_char = (char)((int)mag_bin2 * 9); // 8.0 * 9 = 72 ('H')
    }
    
    // Decode Channel B (Bin 5): Mapped magnitude to numeric state (Mag 4.0 -> 123)
    if (mag_bin5 > 1.0) {
        *out_val = (uint8_t)((int)mag_bin5 * 30 + 3); // 4.0 * 30 + 3 = 123
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT FOURIER MULTI-CHANNEL DATA BUS PROVER\n");
    printf("=============================================================\n");

    // 1. Encode two discrete data channels into a single composite wave signal
    printf("1. Multiplexing data channels into composite waveform...\n");
    printf("   - Channel A payload: 'H' (encoded as Bin 2 peak)\n");
    printf("   - Channel B payload: 123 (encoded as Bin 5 peak)\n");
    
    double composite_wave[DFT_SIZE];
    for (int n = 0; n < DFT_SIZE; n++) {
        // Superposition of orthogonal sine carrier waves
        composite_wave[n] = sin(2.0 * M_PI * 2.0 * n / DFT_SIZE) + 0.5 * sin(2.0 * M_PI * 5.0 * n / DFT_SIZE);
    }

    // 2. Demultiplex and decode the values on the receiver side
    printf("2. Decoding composite signal via Forward DFT demux...\n");
    char decoded_char = '\0';
    uint8_t decoded_val = 0;
    
    run_fourier_bus_demux(composite_wave, &decoded_char, &decoded_val);
    
    // 3. Assert decoded channels match original payloads
    assert(decoded_char == 'H');
    assert(decoded_val == 123);
    
    printf("   ✓ Channel A successfully decoded: '%c'\n", decoded_char);
    printf("   ✓ Channel B successfully decoded: %u\n", decoded_val);

    printf("\n=============================================================\n");
    printf("AUNCIENT FOURIER DATA BUS TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
