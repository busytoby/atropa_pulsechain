#ifndef TSFI_AMMETER_H
#define TSFI_AMMETER_H

#include <stddef.h>

typedef struct {
    // Thermal properties of the heating element
    double thermal_mass;  // m * cp (J/K)
    double k_loss;        // Air cooling loss coefficient (W/K)
    double R0;            // Nominal wire resistance (Ohms)
    double alpha;         // Temperature coefficient of resistance (1/K)
    double T_ambient;     // Ambient reference temperature (K)

    // Mechanical gauge suspension parameters
    double M_needle;      // Moment of inertia (kg*m^2)
    double B_needle;      // Magnetic damping torque (N*m*s)
    double K_spring;      // Mechanical hairspring torque constant (N*m/rad)
    double K_torque;      // Linear expansion to deflection torque (N*m/K)

    // Dynamic state variables
    double T;             // Instantiated wire temperature (K)
    double theta;         // Needle deflection display angle (radians)
    double omega;         // Needle deflection display angular velocity (rad/s)
} TsfiHotWireAmmeter;

// Initialize the Ballantine hot-wire ammeter constants
void tsfi_ammeter_init(TsfiHotWireAmmeter *meter);

// Process a block of RF current samples, returning pointer deflection steps
void tsfi_ammeter_process(
    TsfiHotWireAmmeter *meter,
    const float *input_current_rf,
    float *out_needle_theta,
    size_t count,
    double sample_rate
);

#endif // TSFI_AMMETER_H
