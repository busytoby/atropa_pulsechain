#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Numerical approximation of Bessel Function of first kind J_n(x) using trapezoidal rule
double bessel_j(int n, double x) {
    int steps = 100;
    double sum = 0.0;
    double dt = M_PI / steps;
    
    for (int i = 0; i <= steps; i++) {
        double theta = i * dt;
        double term = cos(n * theta - x * sin(theta));
        if (i == 0 || i == steps) {
            sum += 0.5 * term;
        } else {
            sum += term;
        }
    }
    return sum * dt / M_PI;
}

// Dilemma Synthesizer Gait Profiles
// 0: NOMINAL (stable J0/J1 harmonics)
// 1: FROZEN DILEMMA (gait decaying to a Bessel zero node)
// 2: CHAOTIC JITTER (rapid state transitions)
typedef struct {
    int profile_type;
    double phase;
    double hip_angle;
    double knee_angle;
    bool is_locked;
} DilemmaGait;

void synthesize_bessel_gait(DilemmaGait *gait, double t, double dt) {
    // Phase update driven by special Bessel function J0/J1 modulations
    double omega_mod = 0.0;
    
    if (gait->profile_type == 0) {
        // Nominal Walk: Stable phase velocity modulated by Bessel J0
        omega_mod = 2.0 * M_PI * 1.5 * (1.0 + 0.2 * bessel_j(0, t));
        gait->phase += omega_mod * dt;
    } 
    else if (gait->profile_type == 1) {
        // Frozen Dilemma: Phase velocity decays toward the first zero of J0 (approx x = 2.4048)
        // This locks the gait and traps choice parameters once it falls below threshold
        double decay = bessel_j(0, t * 1.5);
        if (fabs(decay) < 0.05 || gait->is_locked) {
            decay = 0.0;
            gait->is_locked = true;
        }
        omega_mod = 2.0 * M_PI * 1.5 * decay;
        gait->phase += omega_mod * dt;
    } 
    else if (gait->profile_type == 2) {
        // Chaotic Jitter: Rapidly oscillates using high-order J5 harmonics near state boundaries
        omega_mod = 2.0 * M_PI * 1.5 * (1.0 + 8.0 * bessel_j(5, t * 15.0));
        gait->phase += omega_mod * dt;
    }

    // Keep phase in [0, 2*PI]
    if (gait->phase > 2.0 * M_PI) gait->phase -= 2.0 * M_PI;
    if (gait->phase < 0.0) gait->phase += 2.0 * M_PI;

    // Synthesize joint kinematics using Bessel-modulated hip/knee trajectories
    gait->hip_angle = 25.0 * bessel_j(1, gait->phase);
    gait->knee_angle = 50.0 * (0.5 + 0.5 * bessel_j(0, gait->phase * 2.0));
}

int main() {
    printf("=== Auncient Bessel Dilemma Synthesizer Suite ===\n");
    
    DilemmaGait nominal_gait = { .profile_type = 0, .phase = 0.0, .is_locked = false };
    DilemmaGait frozen_gait = { .profile_type = 1, .phase = 0.0, .is_locked = false };
    DilemmaGait chaotic_gait = { .profile_type = 2, .phase = 0.0, .is_locked = false };
    
    double dt = 0.01;
    double sim_time = 3.0; // 3 seconds simulation
    int steps = (int)(sim_time / dt);
    
    printf("[SYNTHESIS] Executing Bessel-modulated gait generation...\n");
    
    double terminal_nominal_var = 0.0;
    double terminal_frozen_var = 0.0;
    
    double last_hip_frozen = 0.0;
    int frozen_ticks = 0;

    for (int i = 0; i < steps; i++) {
        double t = i * dt;
        
        synthesize_bessel_gait(&nominal_gait, t, dt);
        synthesize_bessel_gait(&frozen_gait, t, dt);
        synthesize_bessel_gait(&chaotic_gait, t, dt);
        
        // Track frozen status in the second half of simulation
        if (i > 150) { 
            if (fabs(frozen_gait.hip_angle - last_hip_frozen) < 1.0e-5) {
                frozen_ticks++;
            }
        }
        last_hip_frozen = frozen_gait.hip_angle;
        
        // Accumulate test variances
        terminal_nominal_var += nominal_gait.hip_angle * nominal_gait.hip_angle;
        terminal_frozen_var += frozen_gait.hip_angle * frozen_gait.hip_angle;
        
        if (i % 100 == 0) {
            printf("\n  |- Time %.1fs:\n", t);
            printf("     |- NOMINAL (J0): Hip=%.2f, Knee=%.2f, Phase=%.3f rad, Status=%s\n", 
                   nominal_gait.hip_angle, nominal_gait.knee_angle, nominal_gait.phase,
                   nominal_gait.is_locked ? "LOCKED" : "NOMINAL");
            printf("     |- FROZEN  (J0-decay): Hip=%.2f, Knee=%.2f, Phase=%.3f rad, Status=%s\n", 
                   frozen_gait.hip_angle, frozen_gait.knee_angle, frozen_gait.phase,
                   frozen_gait.is_locked ? "LOCKED" : "NOMINAL");
            printf("     |- CHAOTIC (J5-jitter): Hip=%.2f, Knee=%.2f, Phase=%.3f rad, Status=%s\n", 
                   chaotic_gait.hip_angle, chaotic_gait.knee_angle, chaotic_gait.phase,
                   chaotic_gait.is_locked ? "LOCKED" : "NOMINAL");
        }
    }
    
    printf("\n[Auncient Diagnostics] Verification Results:\n");
    printf("  |- Nominal gait MS power: %.2f\n", terminal_nominal_var / steps);
    printf("  |- Frozen dilemma MS power: %.2f\n", terminal_frozen_var / steps);
    printf("  |- Frozen dilemma lock ticks: %d / 150\n", frozen_ticks);
    
    // Assert that the frozen dilemma gait successfully locked motion (variance drops and stays static)
    assert(frozen_ticks > 50);
    assert(frozen_gait.is_locked == true);
    assert(nominal_gait.is_locked == false);
    
    printf("=== [SUCCESS] Bessel Dilemma Synthesizer successfully verified! ===\n");
    return 0;
}
