#include "../firmware/tsfi_lib/tsfi_alsa_synth.c"
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>

#define DFT_SIZE 16
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Performs DFT and maps resolved peak frequencies directly to ALSA synth
void run_closed_loop_analysis(ALSASynth *synth) {
    double input_real[DFT_SIZE];
    double out_real[DFT_SIZE];
    double out_imag[DFT_SIZE];
    
    // 1. Generate spatial wave signal (superposition of bin 2 and 5)
    for (int n = 0; n < DFT_SIZE; n++) {
        input_real[n] = sin(2.0 * M_PI * 2.0 * n / DFT_SIZE) + 0.5 * sin(2.0 * M_PI * 5.0 * n / DFT_SIZE);
    }
    
    // 2. Perform DFT
    for (int k = 0; k < DFT_SIZE; k++) {
        out_real[k] = 0.0;
        out_imag[k] = 0.0;
        for (int n = 0; n < DFT_SIZE; n++) {
            double angle = 2.0 * M_PI * k * n / DFT_SIZE;
            out_real[k] += input_real[n] * cos(angle);
            out_imag[k] -= input_real[n] * sin(angle);
        }
    }
    
    // 3. Map peak frequencies to the synthesizer
    printf("   [CLOSED LOOP] Analyzing DFT spectrum and shifting synth tones:\n");
    for (int k = 0; k < DFT_SIZE / 2 + 1; k++) {
        double magnitude = sqrt(out_real[k] * out_real[k] + out_imag[k] * out_imag[k]);
        if (magnitude > 1.0) {
            double freq = k * 220.0;
            printf("     - Found Peak Bin %d (Mag: %.2f) -> Set Synth Tone: %.1f Hz\n", k, magnitude, freq);
            
            // Modulate the active synthesizer frequency
            alsa_synth_set_frequency(synth, freq);
            assert(synth->current_frequency == freq);
            
            // Play each tone briefly (0.5 seconds)
            usleep(500000);
        }
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT CLOSED-LOOP FOURIER-ALSA SYNTHESIS VERIFICATION\n");
    printf("=============================================================\n");

    ALSASynth synth;
    
    // Initialize synthesizer thread (defaults to 220.0 Hz)
    printf("1. Launching ALSA Sound Synthesis Modulator...\n");
    bool init_ok = alsa_synth_init(&synth, 220.0);
    assert(init_ok == true);
    printf("   ✓ Synth thread initialized successfully.\n");

    // Run closed-loop DFT-to-audio peak mapping
    printf("2. Running spatial-frequency DFT analysis loop...\n");
    run_closed_loop_analysis(&synth);
    printf("   ✓ Closed-loop frequency modulation completed.\n");

    // Clean stop
    printf("3. Stopping audio synthesis threads...\n");
    alsa_synth_stop(&synth);
    printf("   ✓ System teardown finished cleanly.\n");

    printf("\n=============================================================\n");
    printf("AUNCIENT CLOSED-LOOP AUDIO VERIFICATION TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
