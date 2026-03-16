#ifndef TSFI_DYSNOMIA_CRC_H
#define TSFI_DYSNOMIA_CRC_H

#include "tsfi_dysnomia.h"
#include "tsfi_wave512.h"
#include "tsfi_wave_any.h"

typedef struct {
    wave512 state_vector;
    wave512 salt_vector;
    uint64_t op_counter;
} DysnomiaReactiveState;

// NewState = (OldState * Data) + React(Data, Ichidai, Daiichi)
// Ichidai: Forward Reaction (Eta)
// Daiichi: Inverse Reaction (Kappa)

// Standard Thunk Signature: (Stream, Context/Uniforms)
void __attribute__((weak)) dysnomia_reactive_crc_thunk(const WaveStream* stream, DysnomiaReactiveState* state);

// Extended Thunk (Not standard TSFiOperator compatible without wrapper)
void dysnomia_reactive_crc_dai_thunk(DysnomiaReactiveState* state, const WaveStream* stream, struct Dai* reaction);

void dysnomia_derive_salt(DysnomiaReactiveState* state, struct Fa* profile);

#endif