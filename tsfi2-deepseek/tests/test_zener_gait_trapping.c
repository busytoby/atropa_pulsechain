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

void run_gait_simulation(const char *name, double vs_voltage, double rs_resistance, int *out_class) {
    printf("\n=== Running Simulation: %s ===\n", name);
    
    TsfiZener zener;
    double sample_rate = 1000.0;
    tsfi_zener_init(&zener, 5.6, 0.003, sample_rate);
    
    GaitState gait = {0};
    double base_omega = 2.0 * M_PI * 1.5;
    double dt = 1.0 / sample_rate;
    
    for (int t = 0; t < 2000; t++) {
        double noise = 0.0;
        tsfi_zener_tick(&zener, vs_voltage, rs_resistance, &noise);
        
        if (fabs(noise) > 0.015) {
            gait.is_trapped = true;
            gait.trapped_count++;
        } else {
            gait.is_trapped = false;
        }
        
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
        
        if (t % 500 == 0) {
            printf("  |- t=%.1fs: Hip=%.2f, Knee=%.2f, Noise=%.6fv, Status=%s\n",
                   (double)t * dt, gait.hip_angle, gait.knee_angle, noise,
                   gait.is_trapped ? "TRAPPED" : "NOMINAL");
        }
    }
    
    double trap_ratio = (double)gait.trapped_count / 2000.0;
    double hip_mean = gait.hip_sum / 2000.0;
    double hip_variance = (gait.hip_sum_sq / 2000.0) - (hip_mean * hip_mean);
    double knee_mean = gait.knee_sum / 2000.0;
    double knee_variance = (gait.knee_sum_sq / 2000.0) - (knee_mean * knee_mean);
    
    *out_class = classify_gait(trap_ratio, hip_variance, knee_variance);
    
    printf("  |- Results -> TrapRatio: %.2f%%, HipVar: %.2f, KneeVar: %.2f\n",
           trap_ratio * 100.0, hip_variance, knee_variance);
    printf("  |- Classifier Mode: %d (%s)\n", *out_class,
           (*out_class == 0) ? "STATIC" : ((*out_class == 1) ? "NOMINAL" : "TRAPPED/STUMBLING"));
}

int main() {
    printf("=== Auncient Zener-Gait Mode Classification Suite ===\n");
    
    int class_breakdown = -1;
    int class_forward = -1;
    
    // 1. Simulation under reverse breakdown (chaotic noise, traps active)
    run_gait_simulation("Reverse Avalanche Breakdown (Traps Active)", -12.0, 50.0, &class_breakdown);
    assert(class_breakdown == 2);
    
    // 2. Simulation under forward bias (no avalanche noise, standard smooth gate)
    run_gait_simulation("Forward Bias Conduction (Smooth Walk)", 5.0, 1000.0, &class_forward);
    assert(class_forward == 1);
    
    printf("\n=== [SUCCESS] Classifier validation completed successfully! ===\n");
    return 0;
}
