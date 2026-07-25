#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define NUM_DRAWBARS 8
#define SAMPLE_RATE 44100.0
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


typedef struct {
    double target_frequency;
    double current_frequency;
    double shaft_velocity; // 0.0 to 1.0 (clutch engagement level)
    double clutch_coupling; // Torque coupling coefficient
} tonewheel_t;

typedef struct {
    double levels[NUM_DRAWBARS]; // 0.0 (muted) to 8.0 (max)
    double contact_wear; // Simulated resistor contact drift (marginal check level)

} drawbar_assembly_t;

// Update the tonewheel shaft velocity based on electromagnetic clutch torque
void tick_tonewheel_clutch(tonewheel_t *tw, double dt) {
    // Torque equation: acceleration proportional to slip velocity and coupling strength
    double target_vel = 1.0;
    double slip = target_vel - tw->shaft_velocity;
    double torque = slip * tw->clutch_coupling;
    
    tw->shaft_velocity += torque * dt;
    if (tw->shaft_velocity > 1.0) {
        tw->shaft_velocity = 1.0;
    }
    
    // Scale output frequency by shaft velocity (simulates pitch spin-up)
    tw->current_frequency = tw->target_frequency * tw->shaft_velocity;
}

// Generate sound output by mixing harmonics through worn drawbar contacts
double mix_drawbars_with_grit(const tonewheel_t *tw, const drawbar_assembly_t *db, double time) {
    double mixed_signal = 0.0;
    double base_freq = tw->current_frequency;

    // Harmonic ratios for standard drawbars: sub-octave, 5th, octave, 12th, 15th, etc.
    double ratios[NUM_DRAWBARS] = { 0.5, 1.498, 1.0, 2.0, 2.996, 4.0, 5.039, 6.0 };

    for (int i = 0; i < NUM_DRAWBARS; i++) {
        if (db->levels[i] <= 0.0) continue;

        // Introduce marginal check contact grit/noise based on wear level
        double noise = ((double)rand() / (double)RAND_MAX - 0.5) * db->contact_wear;
        double level_with_grit = db->levels[i] + noise;
        if (level_with_grit < 0.0) level_with_grit = 0.0;

        // Calculate sine wave value for this harmonic
        double phase = 2.0 * M_PI * base_freq * ratios[i] * time;
        mixed_signal += sin(phase) * (level_with_grit / 8.0);
    }

    return mixed_signal;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ELECTROMECHANICAL TONEWHEEL & DRAWBAR AUDIT\n");
    printf("=============================================================\n");

    tonewheel_t tw = {
        .target_frequency = 440.0, // A4
        .current_frequency = 0.0,
        .shaft_velocity = 0.0, // Start stationary
        .clutch_coupling = 5.0 // High torque response
    };

    drawbar_assembly_t db = {
        .levels = { 8.0, 0.0, 8.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, // 16' and 8' active
        .contact_wear = 0.15 // 15% resistor contact instability
    };

    // 1. Verify Pitch spin-up glide (clutch acceleration)
    printf("[TEST] Engaging electromagnetic clutch and checking spin-up...\n");
    double dt = 0.05; // 50ms intervals
    
    // Ticking initial step: should start accelerating
    tick_tonewheel_clutch(&tw, dt);
    assert(tw.shaft_velocity > 0.0);
    assert(tw.current_frequency > 0.0);
    assert(tw.current_frequency < tw.target_frequency);
    printf("   ✓ Spin-up started. Velocity: %.4f | Frequency: %.2f Hz\n", 
           tw.shaft_velocity, tw.current_frequency);

    // Let the shaft accelerate close to target frequency
    for (int i = 0; i < 20; i++) {
        tick_tonewheel_clutch(&tw, dt);
    }
    assert(tw.shaft_velocity > 0.95);
    printf("   ✓ Tonewheel stabilized. Velocity: %.4f | Frequency: %.2f Hz\n", 
           tw.shaft_velocity, tw.current_frequency);

    // 2. Verify Drawbar mixing and wear grit
    printf("[TEST] Calculating drawbar output mixing and check noise...\n");
    double out1 = mix_drawbars_with_grit(&tw, &db, 0.01);
    double out2 = mix_drawbars_with_grit(&tw, &db, 0.01);
    
    // Since wear noise is random, two calls at the same timestamp should yield slightly different signals
    assert(out1 != out2);
    printf("   ✓ Mixed signal output generated. Drawbar contact wear successfully verified.\n");

    printf("=============================================================\n");
    printf("TONEWHEEL CLUTCH & DRAWBAR TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
