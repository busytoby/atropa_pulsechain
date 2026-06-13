#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "tsfi_valve.h"
#include "lau_memory.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NUM_SAMPLES 1024
#define SAMPLING_RATE 44100.0
#define BENCH_RUNS 100000

int main() {
    printf("=== TSFi2 DeForest Grounding Unit Test & Benchmark ===\n");

    // Initialize two identical valves
    TsfiValveTriode standard_ground;
    TsfiValveTriode deforest_ground;

    tsfi_valve_init(&standard_ground, 100.0, 0.00002, 250.0, -1.5);
    tsfi_valve_init(&deforest_ground, 100.0, 0.00002, 250.0, -1.5);

    // Configure deforest_ground to use A+ filament ground
    deforest_ground.use_deforest_ground = 1;
    deforest_ground.V_filament = 5.0; // 5.0V filament potential

    float *vg_in = (float*)lau_memalign(64, NUM_SAMPLES * sizeof(float));
    float *vp_out_std = (float*)lau_memalign(64, NUM_SAMPLES * sizeof(float));
    float *vp_out_def = (float*)lau_memalign(64, NUM_SAMPLES * sizeof(float));

    // Generate 1 kHz sine wave input grid signal (amplitude 0.5 V)
    for (int i = 0; i < NUM_SAMPLES; i++) {
        vg_in[i] = 0.5f * sinf(2.0f * (float)M_PI * 1000.0f * (float)i / (float)SAMPLING_RATE);
    }

    // Process both valves (beta = 0.15 regenerative feedback)
    tsfi_valve_process_regenerative(&standard_ground, vg_in, vp_out_std, NUM_SAMPLES, 0.2, 1.0, 0.15);
    tsfi_valve_process_regenerative(&deforest_ground, vg_in, vp_out_def, NUM_SAMPLES, 0.2, 1.0, 0.15);

    // Verify output voltage shift due to positive filament ground bias offset (V_filament/2 = -2.5V shift)
    float diff_sum = 0.0f;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        diff_sum += fabs(vp_out_std[i] - vp_out_def[i]);
    }
    float avg_diff = diff_sum / NUM_SAMPLES;

    printf("[TEST] Average output deviation due to DeForest Grounding: %.4f V\n", avg_diff);
    if (avg_diff > 1.0f) {
        printf("[SUCCESS] DeForest A+ Grounding bias shift detected and verified!\n");
    } else {
        printf("[FAILURE] DeForest Grounding bias shift was negligible: %.4f V\n", avg_diff);
        return 1;
    }

    // Benchmark Execution Speed
    printf("[BENCHMARK] Executing %d runs of %d samples...\n", BENCH_RUNS, NUM_SAMPLES);
    clock_t start = clock();
    for (int run = 0; run < BENCH_RUNS; run++) {
        tsfi_valve_process_regenerative(&deforest_ground, vg_in, vp_out_def, NUM_SAMPLES, 0.2, 1.0, 0.15);
    }
    clock_t end = clock();
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    printf("[BENCHMARK] Time taken: %.4f seconds (Throughput: %.2f MSamples/sec)\n", 
           time_taken, (double)(NUM_SAMPLES * BENCH_RUNS) / (time_taken * 1e6));

    lau_free(vg_in);
    lau_free(vp_out_std);
    lau_free(vp_out_def);

    return 0;
}