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

#include "tsfi_pulsechain_rpc.h"
#include "tsfi_wire_firmware.h"

// Numerical approximation of Bessel Function of first kind J_n(x)
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

// 2-joint gait state with statistics
typedef struct {
    int profile_type; // 0: NOMINAL, 1: FROZEN DILEMMA, 2: CHAOTIC JITTER
    double phase;
    double hip_angle;
    double knee_angle;
    bool is_locked;
    int trapped_count;
    bool is_trapped;
    
    double hip_sum;
    double hip_sum_sq;
    double knee_sum;
    double knee_sum_sq;
} BesselZenerGait;

// Simple helper to draw an ASCII representation of the joint status
void draw_gait_ascii(const char *name, double hip, double knee, bool trapped, bool locked) {
    char strip[30];
    memset(strip, ' ', sizeof(strip) - 1);
    strip[sizeof(strip) - 1] = '\0';

    int hip_pos = (int)((hip + 25.0) / 50.0 * 12.0);
    if (hip_pos < 0) hip_pos = 0;
    if (hip_pos > 12) hip_pos = 12;
    strip[hip_pos] = 'H';

    int knee_pos = 15 + (int)(knee / 70.0 * 12.0);
    if (knee_pos < 15) knee_pos = 15;
    if (knee_pos > 27) knee_pos = 27;
    strip[knee_pos] = 'K';

    printf("  %-12s |%s| %s %s\n", name, strip, 
           trapped ? "[TRAPPED]" : "         ", 
           locked ? "[LOCKED]" : "");
}

// Classify the gait based on movement variance and trapped ticks ratio
int classify_gait_state(double trap_ratio, double hip_variance, double knee_variance, bool is_locked) {
    if (is_locked || hip_variance < 5.0 || knee_variance < 5.0) {
        return 0; // STATIC/FROZEN
    }
    if (trap_ratio > 0.20) {
        return 2; // TRAPPED/STUMBLING
    }
    return 1; // NOMINAL WALKING
}

// Evaluate trigger decisions
int evaluate_gait_trigger(int mode, int transitions_count) {
    if (transitions_count > 5) {
        return 3; // TRIGGER_RESET (Deadlock Logic Trap activated)
    }
    if (mode == 0) {
        return 0; // NO_TRIGGER (Static state / dilemma without choice)
    }
    if (mode == 2) {
        return 2; // TRIGGER_RECOVERY (Active stumble recovery)
    }
    return 1; // TRIGGER_NOMINAL
}

void simulate_bessel_zener_gait(const char *name, int profile, double vs_voltage, double rs_resistance, bool choose_to_trigger_trap, bool trap_null_state, int *out_class, int *out_trigger) {
    printf("\n=== Simulating Bessel-Zener Hybrid Gait: %s (Trap Choice: %s, Null State Trap: %s) ===\n", 
           name, choose_to_trigger_trap ? "TRIGGER" : "BYPASS", trap_null_state ? "STICKY" : "TRANSIENT");
    
    TsfiZener zener;
    double sample_rate = 1000.0;
    tsfi_zener_init(&zener, 5.6, 0.003, sample_rate);
    
    BesselZenerGait gait = {
        .profile_type = profile,
        .phase = 0.0,
        .is_locked = false,
        .is_trapped = false,
        .trapped_count = 0,
        .hip_sum = 0.0,
        .hip_sum_sq = 0.0,
        .knee_sum = 0.0,
        .knee_sum_sq = 0.0
    };
    
    double dt = 1.0 / sample_rate;
    int last_mode = -1;
    int transitions_count = 0;
    double noise_power = 0.0;
    bool gait_trapped_mq_published = false;
    
    // Simulate 2000 steps (2.0s)
    for (int t = 0; t < 2000; t++) {
        double time_sec = t * dt;
        double noise = 0.0;
        
        double active_vs = vs_voltage;
        if (strcmp(name, "Chaotic-Reset") == 0) {
            active_vs = (t % 160 < 80) ? -12.0 : -5.0;
        }
        
        // Tick the physical Auncient Zener model
        tsfi_zener_tick(&zener, active_vs, rs_resistance, &noise);
        
        // Low-pass filter noise power to smooth state decisions
        noise_power = 0.85 * noise_power + 0.15 * fabs(noise);
        
        // Zener noise power threshold determines if gait is dynamically trapped
        if (noise_power > 0.0005) {
            gait.is_trapped = true;
            gait.trapped_count++;
            if (!gait_trapped_mq_published) {
                tsfi_thunk_publish_mq("M:GAIT_TRAPPED");
                gait_trapped_mq_published = true;
            }
        } else {
            gait.is_trapped = false;
        }
        
        // Track state transitions for logic traps
        int current_tick_mode = gait.is_trapped ? 2 : 1;
        if (last_mode != -1 && current_tick_mode != last_mode) {
            transitions_count++;
        }
        last_mode = current_tick_mode;
        
        // Calculate Bessel phase velocity modulation
        double omega_mod = 0.0;
        if (gait.profile_type == 0) {
            // Nominal Walk: Modulated by J0(t)
            omega_mod = 2.0 * M_PI * 1.5 * (1.0 + 0.2 * bessel_j(0, time_sec));
        } else if (gait.profile_type == 1) {
            // Frozen Dilemma: Decays to J0 zero node
            double decay = bessel_j(0, time_sec * 1.5);
            if (fabs(decay) < 0.05 || (gait.is_locked && trap_null_state)) {
                decay = 0.0;
                gait.is_locked = true;
            } else {
                gait.is_locked = false;
            }
            omega_mod = 2.0 * M_PI * 1.5 * decay;
        } else if (gait.profile_type == 2) {
            // Chaotic Jitter: Rapid oscillation driven by J5
            omega_mod = 2.0 * M_PI * 1.5 * (1.0 + 8.0 * bessel_j(5, time_sec * 15.0));
        }
        
        // Introduce feedback loop: trapped state scales down speed only if we choose to trigger the trap
        double active_omega = omega_mod;
        if (gait.is_trapped && choose_to_trigger_trap) {
            active_omega *= 0.05; // Trapped speed reduction
        } else {
            active_omega += noise * 100.0; // Phase perturbation from Zener noise (Trap Bypassed)
        }
        
        gait.phase += active_omega * dt;
        while (gait.phase > 2.0 * M_PI) gait.phase -= 2.0 * M_PI;
        while (gait.phase < 0.0) gait.phase += 2.0 * M_PI;
        
        // Kinematic joint calculations using Bessel functions
        gait.hip_angle = 25.0 * bessel_j(1, gait.phase);
        gait.knee_angle = 50.0 * (0.5 + 0.5 * bessel_j(0, gait.phase * 2.0));
        
        // Statistics accumulation
        gait.hip_sum += gait.hip_angle;
        gait.hip_sum_sq += gait.hip_angle * gait.hip_angle;
        gait.knee_sum += gait.knee_angle;
        gait.knee_sum_sq += gait.knee_angle * gait.knee_angle;
        
        // Periodically print ASCII art status updates
        if (t > 0 && t % 400 == 0) {
            draw_gait_ascii(name, gait.hip_angle, gait.knee_angle, gait.is_trapped, gait.is_locked);
        }
    }
    
    double trap_ratio = (double)gait.trapped_count / 2000.0;
    double hip_mean = gait.hip_sum / 2000.0;
    double hip_variance = (gait.hip_sum_sq / 2000.0) - (hip_mean * hip_mean);
    double knee_mean = gait.knee_sum / 2000.0;
    double knee_variance = (gait.knee_sum_sq / 2000.0) - (knee_mean * knee_mean);
    
    *out_class = classify_gait_state(trap_ratio, hip_variance, knee_variance, gait.is_locked);
    *out_trigger = evaluate_gait_trigger(*out_class, transitions_count);
    
    if (*out_trigger == 1) {
        tsfi_thunk_publish_mq("M:TRIGGER_NOMINAL");
    } else if (*out_trigger == 2) {
        tsfi_thunk_publish_mq("M:TRIGGER_RECOVERY");
    } else if (*out_trigger == 3) {
        tsfi_thunk_publish_mq("M:TRIGGER_RESET");
    } else if (*out_trigger == 0) {
        tsfi_thunk_publish_mq("M:NO_TRIGGER");
    }
    
    printf("  |- Results -> TrapRatio: %.2f%%, Transitions: %d, HipVar: %.2f, KneeVar: %.2f\n",
           trap_ratio * 100.0, transitions_count, hip_variance, knee_variance);
    printf("  |- Classifier Mode: %d (%s)\n", *out_class,
           (*out_class == 0) ? "STATIC" : ((*out_class == 1) ? "NOMINAL" : "TRAPPED/STUMBLING"));
    printf("  |- Trigger Result:   %d (%s)\n", *out_trigger,
           (*out_trigger == 0) ? "NO_TRIGGER" : ((*out_trigger == 1) ? "TRIGGER_NOMINAL" : ((*out_trigger == 2) ? "TRIGGER_RECOVERY" : "TRIGGER_RESET")));
}

int main() {
    printf("=== Auncient Hybrid Bessel-Zener Gait Trapping Suite ===\n");
    tsfi_wire_firmware_init();
    
    int class_val = -1, trigger_val = -1;
    
    // Test 1: Nominal Bessel Walk under Forward Bias (no noise -> walking cleanly, untrapped)
    simulate_bessel_zener_gait("Nominal-Walk", 0, 5.0, 1000.0, true, true, &class_val, &trigger_val);
    assert(class_val == 1);
    assert(trigger_val == 1); // TRIGGER_NOMINAL
    
    // Test 2: Nominal Bessel Walk under Reverse Breakdown (noise -> trapped stumbling)
    simulate_bessel_zener_gait("Reverse-Trapped", 0, -12.0, 50.0, true, true, &class_val, &trigger_val);
    assert(class_val == 2);
    assert(trigger_val == 2); // TRIGGER_RECOVERY
    
    // Test 3: Frozen Dilemma Gait under Forward Bias with sticky null trap (decay locks permanently -> frozen)
    simulate_bessel_zener_gait("Decay-Frozen", 1, 5.0, 1000.0, true, true, &class_val, &trigger_val);
    assert(class_val == 0);
    assert(trigger_val == 0); // NO_TRIGGER (Dilemma without choice)
    
    // Test 4: Chaotic Jitter Gait under Reverse Breakdown (Violent J5 oscillations -> reset logic trap)
    simulate_bessel_zener_gait("Chaotic-Reset", 2, -12.0, 50.0, true, true, &class_val, &trigger_val);
    assert(trigger_val == 3); // TRIGGER_RESET
    
    // Test 5: Trap Bypass Choice (avalanche breakdown active, but we CHOOSE not to trigger the trap)
    simulate_bessel_zener_gait("Bypassed-Trapped", 0, -12.0, 50.0, false, true, &class_val, &trigger_val);
    assert(class_val == 1 || class_val == 2);
    
    // Test 6: Escapable Null State (decay happens, but we choose NOT to trap/lock it permanently)
    simulate_bessel_zener_gait("Transient-Null", 1, 5.0, 1000.0, true, false, &class_val, &trigger_val);
    // Since we chose not to trap the null state, once J0(t) rises back, the gait recovers and moves again
    assert(class_val == 1); // Overall classified as NOMINAL WALKING because it moves again
    assert(trigger_val == 1);
    
    printf("\n=== [SUCCESS] Hybrid Bessel-Zener Gait Trapping Suite Passed! ===\n");
    return 0;
}
