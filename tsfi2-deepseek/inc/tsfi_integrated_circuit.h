#ifndef TSFI_INTEGRATED_CIRCUIT_H
#define TSFI_INTEGRATED_CIRCUIT_H

#include <stdint.h>

typedef struct {
    float temp_c;            // Temperature in Celsius (default 25.0)
    float c_sub;             // Parasitic substrate capacitance (Farads, e.g. 5e-12)
    float v_substrate;       // Substrate bias voltage (usually 0.0V or ground)
    float v_out;             // Output voltage (Volts)
    int state;               // Logic state (0 or 1)
    float active_charge_pf;  // Accumulated charge on load node in pico-Coulombs
} TsfiMonolithicNorGate;

void tsfi_monolithic_nor_init(TsfiMonolithicNorGate *gate);
int tsfi_monolithic_nor_step(TsfiMonolithicNorGate *gate, int input_a, int input_b, float dt_ns, float *out_v);

#endif /* TSFI_INTEGRATED_CIRCUIT_H */
