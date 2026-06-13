#include "tsfi_ammeter.h"
#include <math.h>

void tsfi_ammeter_init(TsfiHotWireAmmeter *meter) {
    if (!meter) return;
    meter->thermal_mass = 0.015;      // Platinum-Iridium thermal mass
    meter->k_loss = 0.05;            // Thermal conduction loss to air
    meter->R0 = 4.5;                 // 4.5 Ohm nominal heater wire
    meter->alpha = 0.00393;          // Temperature resistance coefficient
    meter->T_ambient = 293.15;       // 20 degrees Celsius ambient
    meter->M_needle = 0.002;         // Pointer moment of inertia
    meter->B_needle = 0.01;          // Pointer eddy damping
    meter->K_spring = 0.5;           // Hairspring restoring constant
    meter->K_torque = 0.0025;        // Expansion torque conversion
    meter->T = 293.15;
    meter->theta = 0.0;
    meter->omega = 0.0;
}

void tsfi_ammeter_process(
    TsfiHotWireAmmeter *meter,
    const float *input_current_rf,
    float *out_needle_theta,
    size_t count,
    double sample_rate
) {
    if (!meter || !input_current_rf || !out_needle_theta || count == 0) return;

    double dt = 1.0 / sample_rate;
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

        // 1. Calculate temperature dependent wire resistance
        double R = R0 * (1.0 + alpha * (T - T_ambient));

        // 2. Solve Joule heating thermal balance:
        // dT/dt = (I^2 * R - k_loss * (T - T_ambient)) / thermal_mass
        double Joule_power = i_rf * i_rf * R;
        double cooling_power = k_loss * (T - T_ambient);
        double dT = (Joule_power - cooling_power) / thermal_mass;
        T += dT * dt;

        // 3. Solve needle second-order display mechanics:
        // M_needle * d^2(theta)/dt^2 + B_needle * d(theta)/dt + K_spring * theta = Torque
        double torque = K_torque * (T - T_ambient);
        double d_omega = (torque - B_needle * omega - K_spring * theta) / M_needle;
        omega += d_omega * dt;
        theta += omega * dt;

        // Clamp physical needle stops
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
