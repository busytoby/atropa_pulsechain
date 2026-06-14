#ifndef TSFI_VALVE_H
#define TSFI_VALVE_H

#include <stdint.h>
#include <stddef.h>

// --- Thermionic Valve (Triode) State Structure ---
typedef struct {
    double mu;     // Amplification factor (scaled by Ichidai / eta)
    double K;      // Perveance parameter (scaled by Daiichi / kappa)
    double Vp;     // Plate voltage supply
    double Vg_bias;// Grid bias voltage
    double C_displacement; // Dielectric displacement charge/polarization state (Q_0/C)
    double epsilon_state;  // Dynamic permittivity state of the air/dielectric boundary
    double ionization_factor; // Scaling of ionization surge (soft vacuum factor)
    double V_ionization;      // Voltage threshold where gas ionization occurs (~30V)
    double R_plate;           // Dynamic plate load resistance (Ohm, e.g. 100000.0)
    double Vp_tuner_offset;   // Fine plate battery step tuner offset (V, e.g. -1.5 to +4.5)
    int is_tubular;           // Geometry flag: 1 for cylindrical (Type T), 0 for flat-plate
    int use_deforest_ground;  // 1 to reference grid/plate potentials to A+ positive filament ground
    double V_filament;        // Filament heating potential in Volts (typically 4.0V to 6.0V)
    double T_junction;        // Cathode Temperature in Kelvin (e.g. 1000K)
    double shot_noise_scale;  // Schottky shot noise coefficient [0.0, 1.0]
    double flicker_noise_scale;// flicker noise coefficient [0.0, 1.0]
    uint64_t noise_seed;      // State for micro-physical noise LCG generator
    double state_vp;          // Persistent plate voltage state for continuous differential solvers
    double state_vk;          // Persistent cathode voltage state for continuous differential solvers
} TsfiValveTriode;


// Initialize the valve parameters
void tsfi_valve_init(TsfiValveTriode *valve, double base_mu, double base_k, double base_vp, double base_bias);

// Process a block of samples using Child's Law / Koren Triode Model (AVX-512 vectorized)
// Parameters:
// - valve: Pointer to valve state
// - vg_in: Array of input grid voltages (input signal)
// - vp_out: Array of output plate voltages (output signal)
// - count: Number of samples to process (must be a multiple of 16 for AVX-512)
// - eta: Ichidai parameter to dynamically modulate amplification
// - kappa: Daiichi parameter to dynamically modulate perveance
void tsfi_valve_process_avx512(
    TsfiValveTriode *valve,
    const float *vg_in,
    float *vp_out,
    size_t count,
    double eta,
    double kappa
);

// Process a block of samples with regenerative feedback (sequential)
void tsfi_valve_process_regenerative(
    TsfiValveTriode *valve,
    const float *vg_in,
    float *vp_out,
    size_t count,
    double eta,
    double kappa,
    double beta
);

// Process a block of samples using RK4 dynamic solver for plate capacitance simulation
void tsfi_valve_process_rk4_dynamic(
    TsfiValveTriode *valve,
    const float *vg_in,
    float *vp_out,
    size_t count,
    double eta,
    double kappa,
    double dt,
    double C_parasitic
);

// Process a block of samples using Heun's method (2nd order Runge-Kutta predictor-corrector)
void tsfi_valve_process_heun_dynamic(
    TsfiValveTriode *valve,
    const float *vg_in,
    float *vp_out,
    size_t count,
    double eta,
    double kappa,
    double dt,
    double C_parasitic
);

// Process a block of samples using Implicit Trapezoidal Method with Newton-Raphson iteration
void tsfi_valve_process_implicit_trapezoidal(
    TsfiValveTriode *valve,
    const float *vg_in,
    float *vp_out,
    size_t count,
    double eta,
    double kappa,
    double dt,
    double C_parasitic
);

// Process a block of samples using a state-space Differential Feedback Valve Solver
void tsfi_valve_process_differential_feedback(
    TsfiValveTriode *valve,
    const float *vg_in,
    float *vp_out,
    size_t count,
    double eta,
    double kappa,
    double dt,
    double C_parasitic,
    double C_cathode,
    double R_cathode,
    double beta
);

#endif // TSFI_VALVE_H
