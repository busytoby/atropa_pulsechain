#ifndef TSFI_PLATED_WIRE_H
#define TSFI_PLATED_WIRE_H

#include <stdint.h>

typedef struct {
    float theta;            // Magnetization angle relative to easy axis (radians, -PI/2 to PI/2)
    float anisotropy_k;     // Anisotropy constant (Oe)
    float magnetization_ms; // Saturation magnetization (Gauss)
    int state;              // Resolved binary state (0 or 1)
} TsfiPlatedWireCell;

// Initializes a plated-wire memory cell
void tsfi_plated_wire_init(TsfiPlatedWireCell *cell);

// Performs a read/write operation on the cell.
// word_current: Current applied to word strap (transverse field, Hard Axis drive)
// bit_current: Current applied through central wire (longitudinal field, Easy Axis steer)
// dt_ns: Simulation step duration (nanoseconds)
// v_sense: Output pointer to receive the induced sense voltage (mV) on the bit line
// Returns the resolved state of the cell (0 or 1)
int tsfi_plated_wire_step(TsfiPlatedWireCell *cell, float word_current, float bit_current, float dt_ns, float *v_sense);

#endif /* TSFI_PLATED_WIRE_H */
