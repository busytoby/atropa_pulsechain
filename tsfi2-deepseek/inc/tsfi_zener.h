#ifndef TSFI_ZENER_H
#define TSFI_ZENER_H

typedef struct {
    double vs;          // Source voltage in Volts
    double rs;          // Series resistance in Ohms
} TsfiZenerCircuit;

// --- Zener Diode Physical Simulation Structure ---
typedef struct {
    double Is;          // Forward saturation current (A)
    double Isz;         // Zener breakdown scale current (A)
    double Vt;          // Thermal voltage (V, typically 0.026V at room temp)
    double N;           // Emission coefficient (typically 1.0 - 2.0)
    double Vz0;         // Nominal Zener breakdown voltage at ambient (V, positive)
    double temp_coeff;  // Temperature coefficient of Vz (V / deg C)
    double temp;        // Junction temperature deviation from ambient (deg C)
    double thermal_res; // Junction thermal resistance (deg C / Watt)
    double cooling_rate;// Cooling decay constant
    double sample_rate; // Sampling rate (Hz)

    // Avalanche Noise Dither
    double noise_amp;   // Base amplitude of Zener avalanche noise

    // Associated circuit parameters
    TsfiZenerCircuit circuit;
} TsfiZener;

// Initialize the Zener diode model parameters
void tsfi_zener_init(TsfiZener *zener, double vz0, double temp_coeff, double sample_rate);

// Computes the dynamic Zener breakdown voltage adjusted for current temperature
double tsfi_zener_get_vz(const TsfiZener *zener);

// Solves the circuit for the Zener diode in series with a resistor Rs and source voltage Vs.
// Returns the solved diode voltage Vd across the Zener terminals.
// Also updates junction temperature and injects avalanche dither noise during breakdown.
double tsfi_zener_tick(TsfiZener *zener, double vs, double rs, double *noise_out);

#endif // TSFI_ZENER_H
