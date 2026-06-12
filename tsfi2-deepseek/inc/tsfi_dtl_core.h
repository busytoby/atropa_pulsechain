#ifndef TSFI_DTL_CORE_H
#define TSFI_DTL_CORE_H

#include <stdint.h>

typedef struct {
    float B;       // Magnetic flux density (-1.0 to 1.0)
    float last_H;  // Magnetic field strength history
    int dir;       // History sweep direction (+1 or -1)
} TsfiMagneticCore;

// DTL Simulation
float tsfi_dtl_nand(float v_in1, float v_in2);

// Magnetic Core Memory Simulation
void tsfi_core_init(TsfiMagneticCore *core);
float tsfi_core_update(TsfiMagneticCore *core, float H_current);

#endif /* TSFI_DTL_CORE_H */
