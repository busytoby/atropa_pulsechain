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

// Define a simple 2-joint gait state
typedef struct {
    double phase;
    double hip_angle;
    double knee_angle;
    int trapped_count;
    bool is_trapped;
    
    // Statistics for classification
    double hip_sum;
    double hip_sum_sq;
    double knee_sum;
    double knee_sum_sq;
} GaitState;

// Simple helper to draw an ASCII strip showing joint angles
void draw_ascii_joint_strip(double hip, double knee, bool trapped) {
    char strip[40];
    memset(strip, ' ', sizeof(strip) - 1);
    strip[sizeof(strip) - 1] = '\0';

    // Normalize hip (-25 to +25) to [0, 18]
    int hip_pos = (int)((hip + 25.0) / 50.0 * 18.0);
    if (hip_pos < 0) hip_pos = 0;
    if (hip_pos > 18) hip_pos = 18;
    strip[hip_pos] = 'H';

    // Normalize knee (0 to 70) to [20, 38]
    int knee_pos = 20 + (int)(knee / 70.0 * 18.0);
    if (knee_pos < 20) knee_pos = 20;
    if (knee_pos > 38) knee_pos = 38;
    strip[knee_pos] = 'K';

    printf("  [ASCII ART] |%s| %s\n", strip, trapped ? "[TRAPPED STATE]" : "");
}

// Gait Mode Classifier
// Mode 0: STATIC/FROZEN, Mode 1: NOMINAL WALKING, Mode 2: TRAPPED/STUMBLING
int classify_gait(double trap_ratio, double hip_variance, double knee_variance) {
    if (hip_variance < 5.0 || knee_variance < 5.0) {
        return 0; // STATIC/FROZEN
    }
    if (trap_ratio > 0.20) {
        return 2; // TRAPPED/STUMBLING
    }
    return 1; // NOMINAL WALKING
}

// Decision Trigger Function
// Returns: 0 = NO_TRIGGER (Dilemma without choice/static), 1 = TRIGGER_NOMINAL, 2 = TRIGGER_RECOVERY, 3 = TRIGGER_RESET (Deadlock)
int evaluate_trigger(int mode, int transitions_count) {
    if (transitions_count > 5) {
        return 3; // TRIGGER_RESET (Deadlock Logic Trap activated due to rapid oscillations)
    }
    if (mode == 0) {
        return 0; // NO_TRIGGER (Static state - dilemma without choice)
    }
    if (mode == 2) {
        return 2; // TRIGGER_RECOVERY (Active stumble corrections)
    }
    return 1; // TRIGGER_NOMINAL
}

void run_gait_simulation(const char *name, double vs_voltage, double rs_resistance, int *out_class, int *out_trigger) {
    printf("\n=== Running Simulation: %s ===\n", name);
    
    TsfiZener zener;
    double sample_rate = 1000.0;
    tsfi_zener_init(&zener, 5.6, 0.003, sample_rate);
    
    GaitState gait = {0};
    double base_omega = 2.0 * M_PI * 1.5;
    if (rs_resistance >= 1.0e9) {
        base_omega = 0.0; // Freeze movement to simulate zero-entropy / lack of choice
    }
    double dt = 1.0 / sample_rate;
    
    int last_mode = -1;
    int transitions_count = 0;
    
    double noise_power = 0.0;
    
    // Simulate 2000 ticks (2.0s)
    for (int t = 0; t < 2000; t++) {
        double noise = 0.0;
        
        // If simulating a jittery transition state, artificially pulse voltage to force rapid state changes
        double active_vs = vs_voltage;
        if (strcmp(name, "Jittery State (Deadlock Logic Trap Active)") == 0) {
            active_vs = (t % 160 < 80) ? -12.0 : -5.0; 
        }
        
        tsfi_zener_tick(&zener, active_vs, rs_resistance, &noise);
        
        // Fast decay filter to catch dynamic transitions
        noise_power = 0.85 * noise_power + 0.15 * fabs(noise);
        
        // Classify tick-to-tick trapped state based on filtered noise power
        if (noise_power > 0.0005) {
            gait.is_trapped = true;
            gait.trapped_count++;
        } else {
            gait.is_trapped = false;
        }
        
        // Track state transitions
        int current_tick_mode = gait.is_trapped ? 2 : 1;
        if (last_mode != -1 && current_tick_mode != last_mode) {
            transitions_count++;
        }
        last_mode = current_tick_mode;
        
        double active_omega = base_omega;
        if (gait.is_trapped) {
            active_omega *= 0.05;
        } else {
            active_omega += noise * 100.0;
        }
        
        gait.phase += active_omega * dt;
        if (gait.phase > 2.0 * M_PI) {
            gait.phase -= 2.0 * M_PI;
        }
        
        gait.hip_angle = 25.0 * sin(gait.phase);
        gait.knee_angle = 35.0 * sin(gait.phase * 2.0 - 0.5) + 35.0;
        
        // Accumulate statistics
        gait.hip_sum += gait.hip_angle;
        gait.hip_sum_sq += gait.hip_angle * gait.hip_angle;
        gait.knee_sum += gait.knee_angle;
        gait.knee_sum_sq += gait.knee_angle * gait.knee_angle;
    }
    
    double trap_ratio = (double)gait.trapped_count / 2000.0;
    double hip_mean = gait.hip_sum / 2000.0;
    double hip_variance = (gait.hip_sum_sq / 2000.0) - (hip_mean * hip_mean);
    double knee_mean = gait.knee_sum / 2000.0;
    double knee_variance = (gait.knee_sum_sq / 2000.0) - (knee_mean * knee_mean);
    
    *out_class = classify_gait(trap_ratio, hip_variance, knee_variance);
    *out_trigger = evaluate_trigger(*out_class, transitions_count);
    
    printf("  |- Results -> TrapRatio: %.2f%%, Transitions: %d, HipVar: %.2f, KneeVar: %.2f\n",
           trap_ratio * 100.0, transitions_count, hip_variance, knee_variance);
    printf("  |- Classifier Mode: %d (%s)\n", *out_class,
           (*out_class == 0) ? "STATIC" : ((*out_class == 1) ? "NOMINAL" : "TRAPPED/STUMBLING"));
    printf("  |- Trigger Result:   %d (%s)\n", *out_trigger,
           (*out_trigger == 0) ? "NO_TRIGGER" : ((*out_trigger == 1) ? "TRIGGER_NOMINAL" : ((*out_trigger == 2) ? "TRIGGER_RECOVERY" : "TRIGGER_RESET")));
}

int main() {
    printf("=== Auncient Zener-Gait Mode Classification & Logic Traps Suite ===\n");
    
    int class_val = -1, trigger_val = -1;
    
    // 1. Simulation under reverse breakdown (chaotic noise, recovery trigger)
    run_gait_simulation("Reverse Avalanche Breakdown (Traps Active)", -12.0, 50.0, &class_val, &trigger_val);
    assert(class_val == 2);
    assert(trigger_val == 2); // TRIGGER_RECOVERY (Transitions must stay under 5 due to low-pass filter)
    
    // 2. Simulation under forward bias (no noise, standard walk trigger)
    run_gait_simulation("Forward Bias Conduction (Smooth Walk)", 5.0, 1000.0, &class_val, &trigger_val);
    assert(class_val == 1);
    assert(trigger_val == 1); // TRIGGER_NOMINAL
    
    // 3. Simulation under zero-entropy static inputs (dilemma without choice, no trigger)
    run_gait_simulation("Zero-Entropy Static Input (FROZEN state)", 0.0, 1.0e9, &class_val, &trigger_val);
    assert(class_val == 0);
    assert(trigger_val == 0); // NO_TRIGGER (Dilemma without choice logic)
    
    // 4. Jittery state simulation (Deadlock Logic Trap active)
    run_gait_simulation("Jittery State (Deadlock Logic Trap Active)", -12.0, 50.0, &class_val, &trigger_val);
    assert(trigger_val == 3); // TRIGGER_RESET (Deadlock Logic Trap successfully caught oscillation)
    
    printf("\n=== [SUCCESS] Classifier and Logic Traps validation completed successfully! ===\n");
    return 0;
}
