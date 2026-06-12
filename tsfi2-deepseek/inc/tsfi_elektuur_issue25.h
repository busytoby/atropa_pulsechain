#ifndef TSFI_ELEKTUUR_ISSUE25_H
#define TSFI_ELEKTUUR_ISSUE25_H

#include <stddef.h>

// Wien-Bridge oscillator simulation state
typedef struct {
    float R1;          // Series resistor (Ohm)
    float C1;          // Series capacitor (Farad)
    float R2;          // Parallel resistor (Ohm)
    float C2;          // Parallel capacitor (Farad)
    float V_series;    // Voltage across series capacitor node
    float V_parallel;  // Voltage across parallel RC node
    float gain;        // Op-amp loop gain
    float ntc_temp;    // Simulated NTC temperature (Celsius)
    float ntc_resistance; // Current NTC resistance
    float V_out;       // Output voltage
} TsfiWienBridgeOscillator;

// Atoom-versterker (Atom Amplifier) simulation state
typedef struct {
    float supply_voltage; // Typically 9V
    float temp_celsius;   // Operating temperature
    float bias_ntc;       // Bias adjustment from thermistor
    float input_gain;
    float crossover_dist; // Estimated crossover distortion metric
} TsfiAtoomVersterker;

// DNA structure for the Biotika-Elektuur synthesis optimization
typedef struct {
    float genes[8]; // [R, C, gain_base, ntc_tau, bias_pt, temp_coeff, drive, feedback]
} TsfiBiotikaDna;

// Core functions
void tsfi_wien_bridge_init(TsfiWienBridgeOscillator *osc, const TsfiBiotikaDna *dna);
void tsfi_wien_bridge_step(TsfiWienBridgeOscillator *osc, float dt);

void tsfi_atoom_versterker_init(TsfiAtoomVersterker *amp, float temp);
float tsfi_atoom_versterker_process(TsfiAtoomVersterker *amp, float input_val);

// Biotika genetic operators
float tsfi_biotika_calculate_crest_factor(const float *buffer, size_t size);
TsfiBiotikaDna tsfi_biotika_crossover(const TsfiBiotikaDna *parent_a, const TsfiBiotikaDna *parent_b);
void tsfi_biotika_mutate(TsfiBiotikaDna *dna, float mutation_rate);

#endif // TSFI_ELEKTUUR_ISSUE25_H
