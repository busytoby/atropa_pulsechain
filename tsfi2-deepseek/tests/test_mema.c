#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_mema.h"

int main() {
    printf("=== TSFi2 & Computer Design Issue #16 MEMA Packaging Test ===\n");

    // Test 1: Electrical Crosstalk vs Distance
    printf("[MEMA] Testing Crosstalk under dense packaging (30 microns trace spacing)...\n");
    TsfiMemaModule dense_module;
    tsfi_mema_init(&dense_module);
    dense_module.trace_distance_um = 30.0f; // Dense spacing

    float dt = 0.1f; // 0.1 ns steps
    float noise = 0.0f;
    int dense_glitch = 0;
    float peak_dense_noise = 0.0f;

    // Simulate 2.0 ns rising edge from 0V to 3.0V on aggressor line
    float v_agg = 0.0f;
    for (int i = 0; i < 40; i++) {
        if (i < 20) {
            v_agg = (i / 20.0f) * 3.0f; // Rise to 3.0V over 2.0 ns
        } else {
            v_agg = 3.0f;
        }
        int fail = tsfi_mema_step(&dense_module, v_agg, dt, &noise);
        if (noise > peak_dense_noise) {
            peak_dense_noise = noise;
        }
        if (fail) dense_glitch = 1;
    }
    printf("[MEMA]   Dense spacing peak noise: %f V (Glitch triggered: %d)\n", peak_dense_noise, dense_glitch);
    assert(peak_dense_noise > dense_module.noise_margin_v);
    assert(dense_glitch == 1);

    printf("[MEMA] Testing Crosstalk under relaxed packaging (75 microns trace spacing)...\n");
    TsfiMemaModule relaxed_module;
    tsfi_mema_init(&relaxed_module);
    relaxed_module.trace_distance_um = 75.0f; // Relaxed spacing

    int relaxed_glitch = 0;
    float peak_relaxed_noise = 0.0f;

    v_agg = 0.0f;
    for (int i = 0; i < 40; i++) {
        if (i < 20) {
            v_agg = (i / 20.0f) * 3.0f;
        } else {
            v_agg = 3.0f;
        }
        int fail = tsfi_mema_step(&relaxed_module, v_agg, dt, &noise);
        if (noise > peak_relaxed_noise) {
            peak_relaxed_noise = noise;
        }
        if (fail) relaxed_glitch = 1;
    }
    printf("[MEMA]   Relaxed spacing peak noise: %f V (Glitch triggered: %d)\n", peak_relaxed_noise, relaxed_glitch);
    assert(peak_relaxed_noise < relaxed_module.noise_margin_v);
    assert(relaxed_glitch == 0);

    // Test 2: Thermal Network Settle
    printf("[MEMA] Simulating Thermal Network Settle...\n");
    TsfiMemaModule thermal_module;
    tsfi_mema_init(&thermal_module);

    // Let the module run for 0.5 ms to settle thermal equilibrium
    float thermal_dt = 100.0f; // 100 ns steps
    for (int i = 0; i < 20000; i++) {
        tsfi_mema_step(&thermal_module, 0.0f, thermal_dt, NULL);
    }

    printf("[MEMA] Steady State Temperatures:\n");
    printf("[MEMA]   Substrate Temp: %f C (Theoretical: 92.5 C)\n", thermal_module.temp_substrate);
    printf("[MEMA]   IC 0 Temp:      %f C (Theoretical: 107.5 C)\n", thermal_module.temp_ic[0]);
    printf("[MEMA]   IC 1 Temp:      %f C (Theoretical: 104.5 C)\n", thermal_module.temp_ic[1]);

    // Verify temperatures are within 1C of theoretical values
    assert(fabsf(thermal_module.temp_substrate - 92.5f) < 1.0f);
    assert(fabsf(thermal_module.temp_ic[0] - 107.5f) < 1.0f);
    assert(fabsf(thermal_module.temp_ic[1] - 104.5f) < 1.0f);

    // Memory sanitation checks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_mema", NULL);

    printf("[PASS] MEMA Packaging simulation verified.\n");
    return 0;
}
