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
    double phase0;  // Sub-bass fundamental pitch phase
    double phase1;  // Formant 1 phase
    double phase2;  // Formant 2 phase
} FormantSynth;

// Numerical approximation of Bessel function of first kind J0(x)
double bessel_j0(double x) {
    int steps = 100;
    double sum = 0.0;
    double dt = M_PI / steps;
    for (int i = 0; i <= steps; i++) {
        double theta = i * dt;
        double term = cos(-x * sin(theta));
        if (i == 0 || i == steps) {
            sum += 0.5 * term;
        } else {
            sum += term;
        }
    }
    return sum * dt / M_PI;
}

// Numerical approximation of modified Bessel function of second kind K0(x)
double bessel_k0(double x) {
    if (x <= 0.0) return 10.0; // Clamp singularity
    int steps = 100;
    double max_t = 8.0;
    double dt = max_t / steps;
    double sum = 0.0;
    for (int i = 0; i <= steps; i++) {
        double t = i * dt;
        double term = exp(-x * cosh(t));
        if (i == 0 || i == steps) {
            sum += 0.5 * term;
        } else {
            sum += term;
        }
    }
    return sum * dt;
}

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
    speech.phase0 = 0.0;
    speech.phase1 = 0.0;
    speech.phase2 = 0.0;
    
    double dt = 1.0 / sample_rate;
    
    printf("[Auncient Fusion] Simulating 1.0 second of unified avalanche-formant synthesis...\n");
    
    int transition_count = 0;
    
    // Simulate 1.0 seconds (8000 samples)
    for (int t = 0; t < 8000; t++) {
        double time_sec = t * dt;
        double noise = 0.0;
        // Keep the diode in heavy breakdown to force self-heating and avalanche noise
        tsfi_zener_tick(&zener, -15.0, 100.0, &noise);
        
        // 1. Vowel Shift driven by Junction Temperature (Thermal Drift)
        double temp_factor = zener.temp / 100.0; // Normalize expected temp rise
        speech.f1_freq = 500.0 + temp_factor * 300.0; // Shifts from 500Hz to 800Hz
        speech.f2_freq = 1000.0 + temp_factor * 500.0; // Shifts from 1000Hz to 1500Hz
        
        // 2. Sub-bass Fundamental Pitch (f0) Modulated by Bessel J0 (Efferent Topicality)
        double f0_base = 80.0; // 80 Hz sub-bass
        double f0_modulated = f0_base * (1.0 + 0.15 * bessel_j0(time_sec * 6.0));
        
        // 3. Formant and Sub-bass Phase Generation
        double active_dt0 = f0_modulated * dt;
        double active_dt1 = speech.f1_freq * dt;
        double active_dt2 = speech.f2_freq * dt;
        
        if (fabs(noise) > 0.015) {
            // Inject high-frequency Zener noise jitter into the phase delta (vocal fry)
            active_dt1 += noise * 20.0;
            active_dt2 += noise * 20.0;
            transition_count++;
        }
        
        // Accumulate phase
        speech.phase0 += 2.0 * M_PI * active_dt0;
        speech.phase1 += 2.0 * M_PI * active_dt1;
        speech.phase2 += 2.0 * M_PI * active_dt2;
        
        // Keep phase in [0, 2*PI]
        if (speech.phase0 > 2.0 * M_PI) speech.phase0 -= 2.0 * M_PI;
        if (speech.phase1 > 2.0 * M_PI) speech.phase1 -= 2.0 * M_PI;
        if (speech.phase2 > 2.0 * M_PI) speech.phase2 -= 2.0 * M_PI;
        
        // 4. Triode-like Release Envelope modulated by Bessel K0
        double amp_scale = 1.0;
        if (time_sec > 0.8) {
            double decay_time = (time_sec - 0.8) * 15.0; // Scale decay speed
            amp_scale = bessel_k0(decay_time + 0.1) / bessel_k0(0.1);
            if (amp_scale > 1.0) amp_scale = 1.0;
        }
        
        // Output sample (combined sub-bass fundamental and Formants 1 & 2)
        double sub_bass = 0.5 * sin(speech.phase0);
        double formant1 = 0.25 * sin(speech.phase1);
        double formant2 = 0.25 * sin(speech.phase2);
        double sample = amp_scale * (sub_bass + formant1 + formant2);
        (void)sample; // Suppress unused warning in this test execution
        
        if (t % 2000 == 0) {
            printf("  |- Sample %d: TempDelta=%.2f deg C, f0=%.2f Hz, F1=%.2f Hz, F2=%.2f Hz, AmpScale=%.4f, Fry=%s\n",
                   t, zener.temp, f0_modulated, speech.f1_freq, speech.f2_freq, amp_scale,
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
