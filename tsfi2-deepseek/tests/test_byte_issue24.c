#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_byte_issue24.h"

int main() {
    printf("=== TSFi2 & BYTE Magazine Issue #24 Simulation Test ===\n");

    // 1. Verify APL operations
    printf("[APL] Testing vector outer product & row sum reduction...\n");
    TsfiAplVec4 v1 = {.data = {1.0f, 2.0f, 3.0f, 4.0f}};
    TsfiAplVec4 v2 = {.data = {2.0f, 3.0f, 4.0f, 5.0f}};

    TsfiAplMat4x4 mat = tsfi_apl_outer_product_mul(&v1, &v2);
    // mat[i][j] = v1[i]*v2[j]
    assert(fabs(mat.data[0][0] - 2.0f) < 1e-5f);
    assert(fabs(mat.data[2][3] - 15.0f) < 1e-5f);

    TsfiAplVec4 reduced;
    tsfi_apl_sum_reduction_rows(&mat, &reduced);
    // reduced[i] = v1[i] * sum(v2) = v1[i] * 14.0f
    assert(fabs(reduced.data[0] - 14.0f) < 1e-5f);
    assert(fabs(reduced.data[3] - 56.0f) < 1e-5f);
    printf("[APL]   APL array operations successfully verified.\n");

    // 2. Verify DC Motor PID Feedback Loop
    printf("[PID] Testing DC motor target alignment...\n");
    TsfiDcMotorPid motor;
    // Set gains: Kp = 3.0, Ki = 0.5, Kd = 0.1
    tsfi_dc_motor_init(&motor, 3.0f, 0.5f, 0.1f);
    motor.target_pos = 1.5708f; // 90 degrees

    // Simulate for 50 steps of 0.05 seconds
    for (int step = 0; step < 50; step++) {
        tsfi_dc_motor_step(&motor, 0.05f);
    }
    printf("[PID]   Final motor pos: %f (Target: %f, final error: %f)\n",
           motor.current_pos, motor.target_pos, fabs(motor.target_pos - motor.current_pos));
    assert(fabs(motor.target_pos - motor.current_pos) < 0.02f);
    printf("[PID]   PID closed-loop convergence verified successfully.\n");

    // 3. Verify Rheinstein Fast Trigonometric Approximations
    printf("[TRIG] Testing fast polynomial approximations...\n");
    float test_angles[3] = {0.0f, 1.0f, -2.5f};
    for (int i = 0; i < 3; i++) {
        float standard_sin = sinf(test_angles[i]);
        float approx_sin = tsfi_rheinstein_sin(test_angles[i]);
        float standard_cos = cosf(test_angles[i]);
        float approx_cos = tsfi_rheinstein_cos(test_angles[i]);

        printf("[TRIG]   Angle: %f -> Sin error: %f, Cos error: %f\n",
               test_angles[i], fabs(standard_sin - approx_sin), fabs(standard_cos - approx_cos));
        
        assert(fabs(standard_sin - approx_sin) < 0.005f);
        assert(fabs(standard_cos - approx_cos) < 0.005f);
    }
    printf("[TRIG]   Transcendental series approximations verified successfully.\n");

    // Memory sanitation checks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_byte_issue24", NULL);

    printf("[PASS] BYTE Magazine Issue #24 simulation verified.\n");
    return 0;
}
