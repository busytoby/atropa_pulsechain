#ifndef TSFI_SLT_H
#define TSFI_SLT_H

#include <stdint.h>

typedef struct {
    // Physical resistor values (Ohms)
    float r_pullup;     // R1: Input pull-up resistor (nominal 4k)
    float r_bias;       // R2: Negative bias resistor to -1.5V (nominal 10k)
    float r_base;       // R3: Transistor base coupling resistor (nominal 2k)
    float r_load;       // R4: Output collector load resistor (nominal 1k)

    // Resistor trimming state
    int trimmed;        // 1 if trimmed to 1% tolerance, 0 if 15% untrimmed

    // Thermal state
    float temp_substrate; // Substrate temperature (C)
    float temp_ambient;   // Ambient temperature (C)
    float heat_capacity;  // C_thermal (J/C)
    float thermal_cond;   // k_thermal (W/C)

    // Electronic charge/state variables
    float q_diode[2];    // Charge storage in input diodes (Coulombs)
    float q_base;        // Charge storage in transistor base (Coulombs)
    float v_out;         // Output voltage (V)
    float t;             // Simulation time accumulator (seconds)
    int state;           // Logic state (0 or 1)
} TsfiSltGate;

// Initializes an SLT gate module with either trimmed (1% tolerance) or untrimmed (15% tolerance) resistors
void tsfi_slt_init(TsfiSltGate *gate, int trimmed);

// Simulates one step of the SLT gate, returns the digital output (0 or 1)
int tsfi_slt_step(TsfiSltGate *gate, float v_in1, float v_in2, float dt_ns);

#endif /* TSFI_SLT_H */
