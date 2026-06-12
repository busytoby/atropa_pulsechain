#ifndef TSFI_PARAMETRON_H
#define TSFI_PARAMETRON_H

#include <stdint.h>

typedef struct {
    float x;             // Oscillator displacement (voltage)
    float dxdt;          // Oscillator velocity
    float phase;         // Extracted phase (radians, 0.0 or PI)
    float excitation;    // Pump amplitude h (0.0 to 2.0)
    float t;             // Simulation time accumulator (seconds)
    int state;           // Resolved binary state (0 or 1)
} TsfiParametronCell;

void tsfi_parametron_init(TsfiParametronCell *cell);
int tsfi_parametron_step(TsfiParametronCell *cell, const float *input_phases, const float *input_amplitudes, int num_inputs, float dt_ns);

#endif /* TSFI_PARAMETRON_H */
