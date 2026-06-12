#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_parametron.h"

#define PI 3.14159265358979323846f

// Helper to run parametron simulation until phase locking settles
int run_parametron_voter(float in1_phase, float in2_phase, float in3_phase) {
    TsfiParametronCell cell;
    tsfi_parametron_init(&cell);

    // Three inputs of equal weak amplitude (0.2V each)
    float phases[3] = { in1_phase, in2_phase, in3_phase };
    float amplitudes[3] = { 0.2f, 0.2f, 0.2f };

    // Set excitation high to trigger parametric oscillation locking
    cell.excitation = 1.8f;

    // Simulate for 10 microseconds (10,000 steps of 1 ns)
    float dt = 1.0f; // 1 ns step
    for (int i = 0; i < 10000; i++) {
        tsfi_parametron_step(&cell, phases, amplitudes, 3, dt);
    }

    return cell.state;
}

int main() {
    printf("=== TSFi2 & Computer Design Issue #12 Parametron Logic Test ===\n");

    // Test 1: Simple majority voter
    printf("[PARAMETRON] Testing 3-input majority voter:\n");
    
    // Majority of (1, 1, 0) -> Should be 1
    int maj_1_1_0 = run_parametron_voter(0.0f, 0.0f, PI);
    printf("[PARAMETRON] Inputs: 1, 1, 0 -> Majority Output: %d\n", maj_1_1_0);
    assert(maj_1_1_0 == 1);

    // Majority of (0, 0, 1) -> Should be 0
    int maj_0_0_1 = run_parametron_voter(PI, PI, 0.0f);
    printf("[PARAMETRON] Inputs: 0, 0, 1 -> Majority Output: %d\n", maj_0_0_1);
    assert(maj_0_0_1 == 0);

    // Test 2: 2-input AND gate (Inputs A, B, and Constant Bias = 0)
    printf("[PARAMETRON] Testing simulated 2-input AND gate (Bias = 0):\n");
    
    // A=0, B=0, Bias=0 -> Output 0
    int and_0_0 = run_parametron_voter(PI, PI, PI);
    assert(and_0_0 == 0);
    
    // A=0, B=1, Bias=0 -> Output 0
    int and_0_1 = run_parametron_voter(PI, 0.0f, PI);
    assert(and_0_1 == 0);
    
    // A=1, B=1, Bias=0 -> Output 1
    int and_1_1 = run_parametron_voter(0.0f, 0.0f, PI);
    assert(and_1_1 == 1);
    printf("[PARAMETRON] AND gate passed: 0&0=%d, 0&1=%d, 1&1=%d\n", and_0_0, and_0_1, and_1_1);

    // Test 3: 2-input OR gate (Inputs A, B, and Constant Bias = 1)
    printf("[PARAMETRON] Testing simulated 2-input OR gate (Bias = 1):\n");
    
    // A=0, B=0, Bias=1 -> Output 0
    int or_0_0 = run_parametron_voter(PI, PI, 0.0f);
    assert(or_0_0 == 0);
    
    // A=0, B=1, Bias=1 -> Output 1
    int or_0_1 = run_parametron_voter(PI, 0.0f, 0.0f);
    assert(or_0_1 == 1);
    
    // A=1, B=1, Bias=1 -> Output 1
    int or_1_1 = run_parametron_voter(0.0f, 0.0f, 0.0f);
    assert(or_1_1 == 1);
    printf("[PARAMETRON] OR gate passed: 0|0=%d, 0|1=%d, 1|1=%d\n", or_0_0, or_0_1, or_1_1);

    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_parametron", NULL);

    printf("[PASS] Parametron phase-locked subharmonic logic verified.\n");
    return 0;
}
