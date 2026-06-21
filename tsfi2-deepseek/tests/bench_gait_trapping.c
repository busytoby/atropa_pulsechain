#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include "tsfi_zener.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define LUT_SIZE 1000
#define LUT_MAX_X 50.0

static double j0_lut[LUT_SIZE];
static double j1_lut[LUT_SIZE];
static double j5_lut[LUT_SIZE];

// Direct numerical integration fallback
double compute_numerical_bessel(int n, double x) {
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

// Initialize Lookup Tables for accelerated execution
void init_bessel_luts() {
    for (int i = 0; i < LUT_SIZE; i++) {
        double x = ((double)i / (LUT_SIZE - 1)) * LUT_MAX_X;
        j0_lut[i] = compute_numerical_bessel(0, x);
        j1_lut[i] = compute_numerical_bessel(1, x);
        j5_lut[i] = compute_numerical_bessel(5, x);
    }
}

// Fast lookup with linear interpolation
double fast_bessel_j(int n, double x) {
    if (x < 0.0) {
        // Symmetry properties of Bessel functions
        double val = fast_bessel_j(n, -x);
        return (n % 2 == 0) ? val : -val;
    }
    if (x >= LUT_MAX_X) {
        return compute_numerical_bessel(n, x);
    }
    
    double pos = (x / LUT_MAX_X) * (LUT_SIZE - 1);
    int idx = (int)pos;
    double frac = pos - idx;
    
    const double *lut = NULL;
    if (n == 0) lut = j0_lut;
    else if (n == 1) lut = j1_lut;
    else if (n == 5) lut = j5_lut;
    else return compute_numerical_bessel(n, x);
    
    return lut[idx] * (1.0 - frac) + lut[idx + 1] * frac;
}

typedef struct {
    double phase;
    bool is_locked;
    bool is_trapped;
} BenchGait;

// Runs a single configuration sweep and prints results
void run_benchmark_sweep(const char *label, int profile, double vs_voltage, double rs_resistance, bool pulse_voltage) {
    TsfiZener zener;
    double sample_rate = 1000.0;
    tsfi_zener_init(&zener, 5.6, 0.003, sample_rate);

    BenchGait gait = { .phase = 0.0, .is_locked = false, .is_trapped = false };
    double dt = 1.0 / sample_rate;
    double noise_power = 0.0;
    
    int trapped_ticks = 0;
    int transitions = 0;
    bool last_trapped = false;
    int lock_tick = -1;

    const int total_ticks = 50000; // Increased to 50k for stable throughput testing

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int t = 0; t < total_ticks; t++) {
        double time_sec = t * dt;
        double noise = 0.0;
        
        // Artificial voltage pulsing to simulate dynamic environments
        double active_vs = vs_voltage;
        if (pulse_voltage) {
            active_vs = (t % 160 < 80) ? vs_voltage : -3.0;
        }

        // Tick Auncient Zener simulation
        tsfi_zener_tick(&zener, active_vs, rs_resistance, &noise);
        noise_power = 0.85 * noise_power + 0.15 * fabs(noise);

        gait.is_trapped = (noise_power > 0.0005);
        if (gait.is_trapped) {
            trapped_ticks++;
        }

        if (t > 0 && gait.is_trapped != last_trapped) {
            transitions++;
        }
        last_trapped = gait.is_trapped;

        // Accelerate Bessel dynamics using LUTs
        double omega_mod = 0.0;
        if (profile == 0) {
            omega_mod = 2.0 * M_PI * 1.5 * (1.0 + 0.2 * fast_bessel_j(0, time_sec));
        } else if (profile == 1) {
            double decay = fast_bessel_j(0, time_sec * 1.5);
            if (fabs(decay) < 0.05 || gait.is_locked) {
                decay = 0.0;
                if (!gait.is_locked) {
                    gait.is_locked = true;
                    lock_tick = t;
                }
            }
            omega_mod = 2.0 * M_PI * 1.5 * decay;
        } else {
            omega_mod = 2.0 * M_PI * 1.5 * (1.0 + 8.0 * fast_bessel_j(5, time_sec * 15.0));
        }

        double active_omega = omega_mod;
        if (gait.is_trapped) {
            active_omega *= 0.05;
        } else {
            active_omega += noise * 100.0;
        }

        gait.phase += active_omega * dt;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
    double throughput = (double)total_ticks / (elapsed_ms / 1000.0);

    #pragma omp critical
    {
        printf("%-20s | %5.1fV | %6.1f | %6.2f%% | %11d | ", 
               label, vs_voltage, rs_resistance, (double)trapped_ticks / total_ticks * 100.0, transitions);
        
        if (lock_tick != -1) {
            printf("%9d | ", lock_tick);
        } else {
            printf("      N/A | ");
        }
        printf("%10.2f MS/s\n", throughput / 1000000.0);
    }
}

int main() {
    printf("=== Improved Auncient Gait Trapping Performance Benchmark ===\n");
    printf("[Auncient Benchmark] Initializing Lookup Tables...\n");
    init_bessel_luts();

    printf("\n%-20s | %6s | %6s | %7s | %11s | %9s | %14s\n", 
           "Gait Config", "Voltage", "Resist", "Trap %", "Transitions", "Lock Tick", "Throughput");
    printf("-----------------------------------------------------------------------------------------------\n");

    // Parallel execution of sweeps to maximize core utility
    #pragma omp parallel sections
    {
        #pragma omp section
        run_benchmark_sweep("Nominal Walk (Fwd)",   0,   5.0, 1000.0, false);
        #pragma omp section
        run_benchmark_sweep("Nominal Walk (Rev)",   0, -12.0,   50.0, false);
        #pragma omp section
        run_benchmark_sweep("Decay-Frozen (Fwd)",   1,   5.0, 1000.0, false);
        #pragma omp section
        run_benchmark_sweep("Decay-Frozen (Rev)",   1, -12.0,   50.0, false);
        #pragma omp section
        run_benchmark_sweep("Chaotic J5 (Fwd)",      2,   5.0, 1000.0, false);
        #pragma omp section
        run_benchmark_sweep("Chaotic J5 (Rev)",      2, -12.0,   50.0, false);
        #pragma omp section
        run_benchmark_sweep("Chaotic J5 (Jitter)",   2, -12.0,   50.0, true);
    }

    printf("-----------------------------------------------------------------------------------------------\n");
    printf("[Auncient Benchmark] Completed successfully.\n");
    return 0;
}
