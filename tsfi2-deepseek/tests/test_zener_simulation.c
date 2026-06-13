#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "tsfi_zener.h"
#include "tsfi_crystal.h"

int main() {
    printf("=== TSFi Zener Diode Physical Simulation Test ===\n");
    
    TsfiZener zener;
    double sample_rate = 44100.0;
    
    // Initialize Zener diode: Vz = 5.6V, temp_coeff = +0.003 V/deg C (standard 5.6V Zener)
    tsfi_zener_init(&zener, 5.6, 0.003, sample_rate);
    
    // --- Test 1: Forward Bias Conduction ---
    // With Vs = +5.0V, Rs = 1000 Ohms, the diode should conduct forward
    // and clamp the voltage across it to around 0.6V to 0.8V.
    double noise = 0.0;
    double vd_forward = tsfi_zener_tick(&zener, 5.0, 1000.0, &noise);
    printf("  [MEASUREMENT] Forward voltage drop (Vs = +5.0V): %.4f V\n", vd_forward);
    printf("  [MEASUREMENT] Forward noise: %.6f V\n", noise);
    assert(vd_forward > 0.5 && vd_forward < 1.5);
    assert(noise == 0.0); // No avalanche noise in forward conduction
    
    // --- Test 2: Reverse Zener Breakdown ---
    // With Vs = -10.0V, Rs = 1000 Ohms, the diode should enter breakdown
    // and clamp the voltage to approx -5.6V.
    tsfi_zener_init(&zener, 5.6, 0.003, sample_rate);
    double vd_breakdown = tsfi_zener_tick(&zener, -10.0, 1000.0, &noise);
    printf("  [MEASUREMENT] Reverse breakdown voltage (Vs = -10.0V): %.4f V\n", vd_breakdown);
    printf("  [MEASUREMENT] Breakdown avalanche noise: %.6f V\n", noise);
    assert(vd_breakdown < -5.4 && vd_breakdown > -5.8);
    assert(fabs(noise) > 0.0); // Noise should be generated in breakdown
    
    // --- Test 3: Thermal Self-Heating & Drift ---
    // Under continuous heavy breakdown current, the junction should heat up
    // and the breakdown voltage magnitude should increase due to the positive temp coefficient.
    tsfi_zener_init(&zener, 5.6, 0.003, sample_rate);
    zener.thermal_res = 400.0; // Increase thermal resistance to allow temperature deviation to exceed 10.0C
    double initial_breakdown = tsfi_zener_tick(&zener, -15.0, 100.0, &noise); // High current
    
    // Let it run for 1 second of simulation time to heat up
    for (int i = 0; i < (int)sample_rate; i++) {
        tsfi_zener_tick(&zener, -15.0, 100.0, &noise);
    }
    
    double heated_breakdown = tsfi_zener_tick(&zener, -15.0, 100.0, &noise);
    printf("  [THERMAL] Initial breakdown voltage: %.4f V\n", initial_breakdown);
    printf("  [THERMAL] Heated junction temp deviation: %.2f deg C\n", zener.temp);
    printf("  [THERMAL] Heated breakdown voltage:  %.4f V\n", heated_breakdown);
    
    // With positive temp coefficient, heated breakdown voltage magnitude increases (meaning it becomes more negative)
    assert(zener.temp > 10.0);
    assert(heated_breakdown < initial_breakdown);
    printf("  -> [PASS] Thermal drift and temperature coefficient shift verified!\n");
    
    // --- Test 4: Compatibility Crystal-to-Zener Interface ---
    printf("[COMPATIBILITY] Testing TsfiCrystal alias mapping to TsfiZener...\n");
    TsfiCrystal crystal;
    tsfi_crystal_init(&crystal, 5.6, 0.003, sample_rate);
    double c_noise = 0.0;
    double vc_forward = tsfi_crystal_tick(&crystal, 5.0, 1000.0, &c_noise);
    printf("  [COMPATIBILITY] Crystal (Zener) forward drop: %.4f V\n", vc_forward);
    assert(vc_forward > 0.5 && vc_forward < 1.5);
    assert(tsfi_crystal_get_vz(&crystal) == tsfi_zener_get_vz((TsfiZener*)&crystal));
    assert(crystal.circuit.vs == 5.0);
    assert(crystal.circuit.rs == 1000.0);
    printf("  -> [PASS] Compatibility Crystal alias and circuit tracking validated!\n");
    
    printf("=== [SUCCESS] Zener diode simulation successfully verified! ===\n");
    return 0;
}
