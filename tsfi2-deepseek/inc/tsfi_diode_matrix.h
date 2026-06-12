#ifndef TSFI_DIODE_MATRIX_H
#define TSFI_DIODE_MATRIX_H

#include <stdint.h>

typedef struct {
    // Electrical parameters
    float v_cc;            // Pull-up supply voltage (V, nominal 5.0V)
    float r_pullup;        // Pull-up resistor (Ohms, nominal 2k)
    float v_f;             // Diode forward voltage drop (V, nominal 0.7V)
    float r_diode;         // Diode forward bulk resistance (Ohms, nominal 50)
    float i_leak_0;        // Diode reverse leakage at 25C (A, nominal 1uA)
    float temp;            // Operating temperature (C)
    
    // Internal state
    float v_outputs[8];    // Volts on the 8 output select lines
    float t;               // Simulation time accumulator (seconds)
} TsfiDiodeMatrix;

// Initializes the 3-to-8 diode matrix decoder
void tsfi_diode_matrix_init(TsfiDiodeMatrix *matrix);

// Simulates one step of the address decoder.
// address: 3-bit binary address input (0 to 7)
// dt_ns: Simulation step duration (nanoseconds)
// Returns the active output select line index (0 to 7)
int tsfi_diode_matrix_step(TsfiDiodeMatrix *matrix, uint8_t address, float dt_ns);

#endif /* TSFI_DIODE_MATRIX_H */
