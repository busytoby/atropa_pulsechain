#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLING_RATE 100000.0 // 100 kHz sampling rate to capture RF envelope and thermal constants
#define DURATION_SEC 0.25      // 250 milliseconds simulation duration
#define NUM_SAMPLES (int)(SAMPLING_RATE * DURATION_SEC)

typedef struct {
    // Thermal properties
    double thermal_mass;  // m * cp (J/K)
    double k_loss;        // Cooling coefficient (W/K)
    double R0;            // Nominal resistance (Ohms)
    double alpha;         // Temperature coefficient of resistance (1/K)
    double T_ambient;     // Ambient temperature (K)

    // Mechanical indicator properties
    double M_needle;      // Moment of inertia (kg*m^2)
    double B_needle;      // Rotational damping (N*m*s)
    double K_spring;      // Spring constant (N*m/rad)
    double K_torque;      // Linear expansion to torque factor

    // State variables
    double T;             // Current temperature (K)
    double theta;         // Needle angle deflection (radians)
    double omega;         // Needle angular velocity (rad/s)
} TsfiHotWireAmmeter;

static void hot_wire_process(
    TsfiHotWireAmmeter *meter,
    const float *input_current_rf,
    float *out_needle_theta,
    size_t count,
    double dt
) {
    double thermal_mass = meter->thermal_mass;
    double k_loss = meter->k_loss;
    double R0 = meter->R0;
    double alpha = meter->alpha;
    double T_ambient = meter->T_ambient;

    double M_needle = meter->M_needle;
    double B_needle = meter->B_needle;
    double K_spring = meter->K_spring;
    double K_torque = meter->K_torque;

    double T = meter->T;
    double theta = meter->theta;
    double omega = meter->omega;

    for (size_t step = 0; step < count; step++) {
        double i_rf = (double)input_current_rf[step];

        // 1. Calculate temperature-dependent wire resistance
        double R = R0 * (1.0 + alpha * (T - T_ambient));

        // 2. Solve thermal differential equation: dT/dt = (I^2 * R - k_loss * (T - T_ambient)) / thermal_mass
        double Joule_power = i_rf * i_rf * R;
        double cooling_power = k_loss * (T - T_ambient);
        double dT = (Joule_power - cooling_power) / thermal_mass;
        T += dT * dt;

        // 3. Solve mechanical needle second-order differential equation:
        // M_needle * d^2(theta)/dt^2 + B_needle * d(theta)/dt + K_spring * theta = K_torque * (T - T_ambient)
        double torque = K_torque * (T - T_ambient);
        double d_omega = (torque - B_needle * omega - K_spring * theta) / M_needle;
        omega += d_omega * dt;
        theta += omega * dt;

        // Clamp physical boundaries (mechanical stops)
        if (theta < 0.0) {
            theta = 0.0;
            omega = 0.0;
        }

        out_needle_theta[step] = (float)theta;
    }

    meter->T = T;
    meter->theta = theta;
    meter->omega = omega;
}

int main() {
    printf("=== TSFi2 Stuart Ballantine Hot-Wire Ammeter Unit Test ===\n");

    TsfiHotWireAmmeter meter;
    meter.thermal_mass = 0.015;
    meter.k_loss = 0.05;
    meter.R0 = 4.5;
    meter.alpha = 0.00393; // typical for metals
    meter.T_ambient = 293.15; // 20C
    meter.M_needle = 0.002;
    meter.B_needle = 0.01;
    meter.K_spring = 0.5;
    meter.K_torque = 0.0025; // 2.5 mN*m per Kelvin temperature delta

    // Init state to ambient
    meter.T = meter.T_ambient;
    meter.theta = 0.0;
    meter.omega = 0.0;

    float *input_rf = (float*)malloc(NUM_SAMPLES * sizeof(float));
    float *out_theta = (float*)malloc(NUM_SAMPLES * sizeof(float));

    // Generate RF current envelope (pulsed 10 kHz signal, amplitude 2.5 Amps, active for first half of duration)
    for (int i = 0; i < NUM_SAMPLES; i++) {
        double t = (double)i / SAMPLING_RATE;
        if (t < DURATION_SEC / 2.0) {
            input_rf[i] = 2.5f * (float)sin(2.0 * M_PI * 10000.0 * t);
        } else {
            input_rf[i] = 0.0f;
        }
    }

    double dt = 1.0 / SAMPLING_RATE;
    hot_wire_process(&meter, input_rf, out_theta, NUM_SAMPLES, dt);

    // Verify thermal integration & mechanical damping lag
    // Peak temperature deflection should happen *after* the RF input is turned off
    int rf_off_index = NUM_SAMPLES / 2;
    double theta_at_rf_off = out_theta[rf_off_index];
    
    double max_theta = 0.0;
    int max_idx = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        if (out_theta[i] > max_theta) {
            max_theta = out_theta[i];
            max_idx = i;
        }
    }

    printf("[TEST] Needle angle at RF shutdown: %.4f rad\n", theta_at_rf_off);
    printf("[TEST] Peak Needle angle achieved:  %.4f rad (at step %d / %d)\n", max_theta, max_idx, NUM_SAMPLES);

    // Assert that the peak happens after the input drops (thermal inertia lag)
    if (max_idx > rf_off_index) {
        printf("[SUCCESS] Stuart Ballantine hot-wire thermal integration and mechanical inertia lag verified!\n");
    } else {
        printf("[FAILURE] Failed to capture thermal / mechanical integration lag.\n");
        free(input_rf); free(out_theta);
        return 1;
    }

    free(input_rf); free(out_theta);
    return 0;
}
