#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "tsfi_zener.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 2-Formant Speech Synthesizer State
typedef struct {
    double f1_freq; // First formant (Vowel height)
    double f2_freq; // Second formant (Vowel backness)
    double sample_rate;
    double phase1;
    double phase2;
} FormantSynth;

int main() {
    printf("=== Auncient Zener-Formant Fusion Integration Test ===\n");
    
    TsfiZener zener;
    double sample_rate = 8000.0; // 8 kHz for speech synthesis
    
    // Initialize Zener with Vz = 5.6V and positive temperature coefficient
    tsfi_zener_init(&zener, 5.6, 0.003, sample_rate);
    zener.thermal_res = 800.0; // High thermal resistance to force visible heating
    
    FormantSynth speech;
    speech.f1_freq = 500.0; // Vowel sound /u/ (F1=500Hz, F2=1000Hz)
    speech.f2_freq = 1000.0;
    speech.sample_rate = sample_rate;
    speech.phase1 = 0.0;
    speech.phase2 = 0.0;
    
    double dt = 1.0 / sample_rate;
    
    printf("[Auncient Fusion] Simulating 1.0 second of unified avalanche-formant synthesis...\n");
    
    int transition_count = 0;
    
    // Simulate 1.0 seconds (8000 samples)
    for (int t = 0; t < 8000; t++) {
        double noise = 0.0;
        // Keep the diode in heavy breakdown to force self-heating and avalanche noise
        tsfi_zener_tick(&zener, -15.0, 100.0, &noise);
        
        // 1. Vowel Shift driven by Junction Temperature (Thermal Drift)
        // Vz shifts up as temperature increases, which modulates Formant 1 and Formant 2
        double temp_factor = zener.temp / 100.0; // Normalize expected temp rise
        speech.f1_freq = 500.0 + temp_factor * 300.0; // Shifts from 500Hz to 800Hz (morphing vowel /u/ to /a/)
        speech.f2_freq = 1000.0 + temp_factor * 500.0; // Shifts from 1000Hz to 1500Hz
        
        // 2. Chaotic Friction Injection (Vocal Fry) driven by Avalanche Noise spikes
        // If avalanche noise is high, it modulates the pitch/phases with high frequency jitter
        double active_dt1 = speech.f1_freq * dt;
        double active_dt2 = speech.f2_freq * dt;
        
        if (fabs(noise) > 0.015) {
            // Inject high-frequency Zener noise jitter into the phase delta (vocal fry)
            active_dt1 += noise * 20.0;
            active_dt2 += noise * 20.0;
            transition_count++;
        }
        
        // Accumulate phase
        speech.phase1 += 2.0 * M_PI * active_dt1;
        speech.phase2 += 2.0 * M_PI * active_dt2;
        
        // Keep phase in [0, 2*PI]
        if (speech.phase1 > 2.0 * M_PI) speech.phase1 -= 2.0 * M_PI;
        if (speech.phase2 > 2.0 * M_PI) speech.phase2 -= 2.0 * M_PI;
        
        // Output sample (combined two formants)
        double sample = 0.5 * sin(speech.phase1) + 0.5 * sin(speech.phase2);
        (void)sample; // Suppress unused warning in this test execution
        
        if (t % 2000 == 0) {
            printf("  |- Sample %d: TempDelta=%.2f deg C, Vz=%.4f V, F1=%.2f Hz, F2=%.2f Hz, FryState=%s\n",
                   t, zener.temp, tsfi_zener_get_vz(&zener), speech.f1_freq, speech.f2_freq,
                   (fabs(noise) > 0.015) ? "ACTIVE" : "NOMINAL");
        }
    }
    
    printf("[Auncient Fusion] Synthesis Metrics:\n");
    printf("  |- Total active vocal fry frames from avalanche spikes: %d / 8000\n", transition_count);
    printf("  |- Terminal Vowel Formants: F1=%.2f Hz, F2=%.2f Hz\n", speech.f1_freq, speech.f2_freq);
    
    // Assert successful fusion parameters
    assert(transition_count > 0);
    assert(speech.f1_freq > 550.0); // Verifies thermal shift occurred
    
    printf("=== [SUCCESS] Avalanche-Formant fusion test verified successfully! ===\n");
    return 0;
}
