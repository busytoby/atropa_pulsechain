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


#endif // TSFI_VALVE_H
