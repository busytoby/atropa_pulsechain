#ifndef TSFI_SCHMITT_H
#define TSFI_SCHMITT_H

#include <stdint.h>

typedef struct {
    // Passive component values (Ohms)
    float r_c1;        // Q1 collector resistor (nominal 3k)
    float r_c2;        // Q2 collector resistor / output load (nominal 1k)
    float r_e;         // Common emitter resistor (nominal 1k)
    float r_1;         // Q1 collector to Q2 base coupling resistor (nominal 5k)
    float r_2;         // Q2 base to ground bias resistor (nominal 5k)

    // Voltages
    float v_cc;        // Supply voltage (V, typically 5.0V)
    float v_be_on;     // Transistor base-emitter turn-on (V, typically 0.7V)
    
    // Internal states
    float v_e;         // Shared emitter voltage (V)
    float v_out;       // Output voltage at collector of Q2 (V)
    int q1_state;      // 1 if Q1 is conducting, 0 otherwise
    int q2_state;      // 1 if Q2 is conducting, 0 otherwise
    float t;           // Simulation time accumulator (seconds)
} TsfiSchmittTrigger;

// Initializes the Schmitt trigger with nominal resistor and voltage values
void tsfi_schmitt_init(TsfiSchmittTrigger *trigger);

// Performs a step update of the Schmitt trigger logic.
// v_in: Input analog signal (V)
// dt_ns: Simulation step size (nanoseconds)
// Returns the digital state of the output (0 if output is LOW, 1 if output is HIGH)
int tsfi_schmitt_step(TsfiSchmittTrigger *trigger, float v_in, float dt_ns);

#endif /* TSFI_SCHMITT_H */
