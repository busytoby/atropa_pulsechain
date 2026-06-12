#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_slt.h"

int main() {
    printf("=== TSFi2 & Computer Design Issue #13 Solid Logic Technology (SLT) Test ===\n");

    // Test 1: Functional check of trimmed gate (NAND logic)
    printf("[SLT] Initializing trimmed gate...\n");
    TsfiSltGate trimmed_gate;
    tsfi_slt_init(&trimmed_gate, 1);

    // Apply inputs A=0V, B=0V
    tsfi_slt_step(&trimmed_gate, 0.2f, 0.2f, 100.0f); // 100 ns simulation
    printf("[SLT] Inputs: 0.2V, 0.2V -> V_out: %fV (State: %d)\n", trimmed_gate.v_out, trimmed_gate.state);
    assert(trimmed_gate.state == 1);
    assert(trimmed_gate.v_out > 2.5f);

    // Apply inputs A=3.0V, B=0.2V
    tsfi_slt_step(&trimmed_gate, 3.0f, 0.2f, 100.0f);
    printf("[SLT] Inputs: 3.0V, 0.2V -> V_out: %fV (State: %d)\n", trimmed_gate.v_out, trimmed_gate.state);
    assert(trimmed_gate.state == 1);
    
    // Apply inputs A=3.0V, B=3.0V (transistor turns ON, output goes LOW)
    // Run for 500 ns to allow outputs to stabilize
    for (int i = 0; i < 10; i++) {
        tsfi_slt_step(&trimmed_gate, 3.0f, 3.0f, 50.0f);
    }
    printf("[SLT] Inputs: 3.0V, 3.0V -> V_out: %fV (State: %d)\n", trimmed_gate.v_out, trimmed_gate.state);
    assert(trimmed_gate.state == 0);
    assert(trimmed_gate.v_out < 0.5f);

    // Test 2: Resistor Trim Margin Test under high thermal load
    printf("[SLT] Initiating thermal stress test on trimmed vs untrimmed gates...\n");
    TsfiSltGate untrimmed_gate;
    tsfi_slt_init(&untrimmed_gate, 0);

    // Set substrate temperature to 85C (high stress)
    trimmed_gate.temp_substrate = 85.0f;
    untrimmed_gate.temp_substrate = 85.0f;

    // Simulate both gates with inputs A=3.0V, B=3.0V (transistor trying to turn ON)
    for (int i = 0; i < 20; i++) {
        tsfi_slt_step(&trimmed_gate, 3.0f, 3.0f, 50.0f);
        tsfi_slt_step(&untrimmed_gate, 3.0f, 3.0f, 50.0f);
    }

    printf("[SLT] Thermal Stress (85C) Results:\n");
    printf("[SLT]   Trimmed Gate (1%% tolerance) V_out: %fV (State: %d)\n", trimmed_gate.v_out, trimmed_gate.state);
    printf("[SLT]   Untrimmed Gate (15%% tolerance) V_out: %fV (State: %d)\n", untrimmed_gate.v_out, untrimmed_gate.state);

    // Trimmed gate must succeed in maintaining a solid logic 0
    assert(trimmed_gate.state == 0);
    assert(trimmed_gate.v_out < 0.5f);

    // Untrimmed gate must degrade or fail to remain fully saturated under high heat
    // due to smaller base-drive margins (V_out is higher than trimmed or switches state)
    assert(untrimmed_gate.v_out > trimmed_gate.v_out);
    printf("[SLT] Verification passed: Untrimmed gate showed worse output saturation voltage under thermal load.\n");

    // Memory sanitation checks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_slt", NULL);

    printf("[PASS] IBM Solid Logic Technology (SLT) simulation verified.\n");
    return 0;
}
