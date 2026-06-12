#ifndef TSFI_BYTE_ISSUE24_H
#define TSFI_BYTE_ISSUE24_H

#include <stdint.h>
#include <stdbool.h>

#define APL_VEC_SIZE 4

// APL Array representation
typedef struct {
    float data[APL_VEC_SIZE];
} TsfiAplVec4;

typedef struct {
    float data[APL_VEC_SIZE][APL_VEC_SIZE];
} TsfiAplMat4x4;

// DC Motor PID State
typedef struct {
    float target_pos;      // Target angle (radians)
    float current_pos;     // Current angle (radians)
    float velocity;        // Angular velocity (rad/s)
    float integral;        // PID integral term
    float prev_error;      // Previous step error
    float Kp;              // Proportional gain
    float Ki;              // Integral gain
    float Kd;              // Derivative gain
} TsfiDcMotorPid;

// --- API Declarations ---

// APL Operations
TsfiAplMat4x4 tsfi_apl_outer_product_mul(const TsfiAplVec4 *v1, const TsfiAplVec4 *v2);
void tsfi_apl_sum_reduction_rows(const TsfiAplMat4x4 *mat, TsfiAplVec4 *out_vec);

// DC Motor PID Controller
void tsfi_dc_motor_init(TsfiDcMotorPid *motor, float kp, float ki, float kd);
void tsfi_dc_motor_step(TsfiDcMotorPid *motor, float dt);

// Rheinstein Fast Approximations
float tsfi_rheinstein_sin(float x);
float tsfi_rheinstein_cos(float x);

#endif // TSFI_BYTE_ISSUE24_H
