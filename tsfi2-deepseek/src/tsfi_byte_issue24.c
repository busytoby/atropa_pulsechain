#include "tsfi_byte_issue24.h"
#include <math.h>

// APL Operations
TsfiAplMat4x4 tsfi_apl_outer_product_mul(const TsfiAplVec4 *v1, const TsfiAplVec4 *v2) {
    TsfiAplMat4x4 mat;
    if (!v1 || !v2) return mat;
    for (int i = 0; i < APL_VEC_SIZE; i++) {
        for (int j = 0; j < APL_VEC_SIZE; j++) {
            mat.data[i][j] = v1->data[i] * v2->data[j];
        }
    }
    return mat;
}

void tsfi_apl_sum_reduction_rows(const TsfiAplMat4x4 *mat, TsfiAplVec4 *out_vec) {
    if (!mat || !out_vec) return;
    for (int i = 0; i < APL_VEC_SIZE; i++) {
        float sum = 0.0f;
        for (int j = 0; j < APL_VEC_SIZE; j++) {
            sum += mat->data[i][j];
        }
        out_vec->data[i] = sum;
    }
}

// DC Motor PID Controller
void tsfi_dc_motor_init(TsfiDcMotorPid *motor, float kp, float ki, float kd) {
    if (!motor) return;
    motor->target_pos = 0.0f;
    motor->current_pos = 0.0f;
    motor->velocity = 0.0f;
    motor->integral = 0.0f;
    motor->prev_error = 0.0f;
    motor->Kp = kp;
    motor->Ki = ki;
    motor->Kd = kd;
}

void tsfi_dc_motor_step(TsfiDcMotorPid *motor, float dt) {
    if (!motor || dt <= 0.0f) return;

    float error = motor->target_pos - motor->current_pos;
    motor->integral += error * dt;
    float derivative = (error - motor->prev_error) / dt;

    float torque = motor->Kp * error + motor->Ki * motor->integral + motor->Kd * derivative;

    // Motor mechanical simulation parameters
    float J = 0.1f;   // Moment of inertia
    float B = 0.05f;  // Friction coefficient

    // acceleration = (torque - friction) / inertia
    float acc = (torque - B * motor->velocity) / J;
    motor->velocity += acc * dt;
    motor->current_pos += motor->velocity * dt;

    motor->prev_error = error;
}

// Rheinstein Fast Approximations
// Maclaurin series approximation after wrapping angle to [-pi, pi]
float tsfi_rheinstein_sin(float x) {
    // Wrap to [-pi, pi]
    float pi = 3.14159265f;
    float two_pi = 6.2831853f;
    x = fmodf(x + pi, two_pi);
    if (x < 0.0f) x += two_pi;
    x -= pi;

    float x2 = x * x;
    // Maclaurin approximation: x - x^3/6 + x^5/120 - x^7/5040
    float x3 = x2 * x;
    float x5 = x3 * x2;
    float x7 = x5 * x2;
    float x9 = x7 * x2;

    return x - (x3 / 6.0f) + (x5 / 120.0f) - (x7 / 5040.0f) + (x9 / 362880.0f);
}

float tsfi_rheinstein_cos(float x) {
    // Wrap to [-pi, pi]
    float pi = 3.14159265f;
    float two_pi = 6.2831853f;
    x = fmodf(x + pi, two_pi);
    if (x < 0.0f) x += two_pi;
    x -= pi;

    float x2 = x * x;
    // Maclaurin approximation: 1 - x^2/2 + x^4/24 - x^6/720 + x^8/40320 - x^10/3628800
    float x4 = x2 * x2;
    float x6 = x4 * x2;
    float x8 = x6 * x2;
    float x10 = x8 * x2;

    return 1.0f - (x2 / 2.0f) + (x4 / 24.0f) - (x6 / 720.0f) + (x8 / 40320.0f) - (x10 / 3628800.0f);
}
