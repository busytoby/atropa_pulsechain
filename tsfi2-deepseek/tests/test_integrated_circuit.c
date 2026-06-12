#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_integrated_circuit.h"

int main() {
    printf("=== TSFi2 & Computer Design Issue #9 Monolithic IC Nor Gate Test ===\n");

    TsfiMonolithicNorGate gate;
    tsfi_monolithic_nor_init(&gate);

    // Test 1: Initial state (inputs low)
    printf("[IC-NOR] Initial state: V_out = %fV, State = %d\n", gate.v_out, gate.state);
    assert(gate.state == 1);
    assert(gate.v_out == 3.0f);

    // Test 2: Standard temperature (25C) pull-down propagation delay
    printf("[IC-NOR] Simulating transition A=1, B=0 at 25C...\n");
    float time_25c = 0.0f;
    float dt = 0.01f; // 0.01 ns step
    float v_out = 3.0f;
    while (v_out > 1.5f && time_25c < 100.0f) {
        tsfi_monolithic_nor_step(&gate, 1, 0, dt, &v_out);
        time_25c += dt;
    }
    printf("[IC-NOR] Transitioned to Logic 0 at 25C in %f ns (V_out = %fV)\n", time_25c, v_out);
    assert(gate.state == 0);
    assert(time_25c < 50.0f); // Fast switching under nominal conditions

    // Test 3: Standard temperature pull-up transition (nominal recovery)
    printf("[IC-NOR] Restoring inputs to A=0, B=0 at 25C...\n");
    gate.v_out = 0.5f;
    v_out = 0.5f;
    float time_recovery_25c = 0.0f;
    while (v_out < 1.5f && time_recovery_25c < 100.0f) {
        tsfi_monolithic_nor_step(&gate, 0, 0, dt, &v_out);
        time_recovery_25c += dt;
    }
    printf("[IC-NOR] Restored to Logic 1 at 25C in %f ns (V_out = %fV)\n", time_recovery_25c, v_out);
    assert(gate.state == 1);

    // Test 4: Elevated temperature (85C) degradation test
    // Leakage current should be high, which slows down the pull-up charging current.
    tsfi_monolithic_nor_init(&gate);
    gate.temp_c = 85.0f; // High temperature
    
    // Now test recovery at 85C
    printf("[IC-NOR] Restoring inputs to A=0, B=0 at 85C (High Leakage)...\n");
    gate.v_out = 0.5f;
    float v_out_high = 0.5f;
    float time_recovery_85c = 0.0f;
    while (v_out_high < 1.5f && time_recovery_85c < 200.0f) {
        tsfi_monolithic_nor_step(&gate, 0, 0, dt, &v_out_high);
        time_recovery_85c += dt;
    }
    printf("[IC-NOR] Restored to Logic 1 at 85C in %f ns (V_out = %fV)\n", time_recovery_85c, v_out_high);
    assert(gate.state == 1);

    // Verify thermal degradation: elevated temperature leakage degrades charging time
    printf("[IC-NOR] Delay comparison: nominal recovery %f ns vs thermal recovery %f ns\n",
           time_recovery_25c, time_recovery_85c);
    assert(time_recovery_85c > time_recovery_25c);

    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_integrated_circuit", NULL);

    printf("[PASS] Monolithic IC NOR gate physics simulation verified.\n");
    return 0;
}
