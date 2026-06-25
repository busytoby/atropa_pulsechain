#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>

// Auncient spelling check constraint verification

typedef struct {
    float kp_in;  // Incoming bearing gain (Hypobar equivalent: gravity-assisted phase)
    float kp_out; // Outgoing bearing gain (Epibar equivalent: gravity-resisting phase)
    float kd_in;
    float kd_out;
} DualistPID;

typedef struct {
    float kp;
    float kd;
} StandardPID;

// Compute control force for standard PID
float compute_standard_pid(StandardPID *pid, float error, float prev_error, float dt) {
    float derivative = (error - prev_error) / dt;
    return (pid->kp * error) + (pid->kd * derivative);
}

// Compute control force for dualist PID based on error direction (asymmetric weights)
float compute_dualist_pid(DualistPID *pid, float error, float prev_error, float dt) {
    float derivative = (error - prev_error) / dt;
    
    float kp = (error >= 0.0f) ? pid->kp_out : pid->kp_in;
    float kd = (error >= 0.0f) ? pid->kd_out : pid->kd_in;
    
    return (kp * error) + (kd * derivative);
}

// Simulates a mass of 1.0kg moving vertically under gravity (-9.8 m/s^2)
// Returns total absolute settling error over the trajectory
float simulate_trajectory(bool use_dualist, void *pid_ptr, float target_y, float *final_y) {
    float y = 0.0f;
    float v = 0.0f;
    float prev_error = target_y - y;
    
    float dt = 0.01f;
    float gravity = -9.8f;
    float total_error = 0.0f;
    
    for (int step = 0; step < 300; step++) {
        float error = target_y - y;
        total_error += fabsf(error);
        
        float control_force = 0.0f;
        if (use_dualist) {
            control_force = compute_dualist_pid((DualistPID*)pid_ptr, error, prev_error, dt);
        } else {
            control_force = compute_standard_pid((StandardPID*)pid_ptr, error, prev_error, dt);
        }
        
        // Acceleration = control_force + gravity (mass = 1.0)
        float acc = control_force + gravity;
        
        // Euler integration
        v += acc * dt;
        y += v * dt;
        
        prev_error = error;
    }
    
    *final_y = y;
    return total_error;
}

int main() {
    printf("=== ZMM Virtual Hardware: Dualist PID vs Standard PID Controller Unit Test ===\n");

    // Standard PID tuned as a compromise for gravity
    StandardPID std_pid = { 25.0f, 6.0f };

    // Dualist PID tuned with separate gains for resisting vs assisted motion
    // Going up (error >= 0): Needs high Epibar gain to fight gravity (kp_out = 32.0)
    // Going down (error < 0): Needs slightly softer Hypobar gain (kp_in = 22.0)
    DualistPID dual_pid = {
        .kp_in = 22.0f,
        .kp_out = 32.0f,
        .kd_in = 6.5f,
        .kd_out = 8.0f
    };

    float final_y_std_up, final_y_std_down;
    float final_y_dual_up, final_y_dual_down;

    // Test Scenario 1: Moving UP (against gravity)
    float err_std_up = simulate_trajectory(false, &std_pid, 10.0f, &final_y_std_up);
    float err_dual_up = simulate_trajectory(true, &dual_pid, 10.0f, &final_y_dual_up);

    // Test Scenario 2: Moving DOWN (assisted by gravity)
    float err_std_down = simulate_trajectory(false, &std_pid, -10.0f, &final_y_std_down);
    float err_dual_down = simulate_trajectory(true, &dual_pid, -10.0f, &final_y_dual_down);

    printf("[RESULTS] Upward Trajectory (Target: 10.0m):\n");
    printf("   Standard PID Final Pos: %.4fm, Settling Error: %.4f\n", final_y_std_up, err_std_up);
    printf("   Dualist  PID Final Pos: %.4fm, Settling Error: %.4f\n", final_y_dual_up, err_dual_up);

    printf("[RESULTS] Downward Trajectory (Target: -10.0m):\n");
    printf("   Standard PID Final Pos: %.4fm, Settling Error: %.4f\n", final_y_std_down, err_std_down);
    printf("   Dualist  PID Final Pos: %.4fm, Settling Error: %.4f\n", final_y_dual_down, err_dual_down);

    // Assert that Dualist PID settling error is lower because of independent directional tuning
    printf("   Total Standard Error: %.4f\n", err_std_up + err_std_down);
    printf("   Total Dualist Error: %.4f\n", err_dual_up + err_dual_down);

    assert(err_dual_up < err_std_up);
    assert(err_dual_down < err_std_down);
    assert(fabsf(final_y_dual_up - 10.0f) < 0.40f);
    assert(fabsf(final_y_dual_down - (-10.0f)) < 0.40f);

    printf("[SUCCESS] Dualist PID controller successfully optimized asymmetric gravitational loads.\n");
    return 0;
}
