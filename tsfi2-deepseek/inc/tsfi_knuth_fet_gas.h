#ifndef TSFI_KNUTH_FET_GAS_H
#define TSFI_KNUTH_FET_GAS_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    float v_gate_charge;      // Real Part (FET Charge / Base Gas)
    float v_gate_discharge_v; // Imaginary Part (FET Verlet Decay / Gas Refund)
    uint64_t packed_base2i;
    uint32_t net_gas_cost;
} tsfi_knuth_fet_gas_state_t;

/* Step Knuth Base 2i FET Discharge Physics cycle and calculate net EVM gas cost */
int tsfi_knuth_fet_gas_step(
    float initial_vgate,
    float dt,
    tsfi_knuth_fet_gas_state_t *out_state
);

#endif // TSFI_KNUTH_FET_GAS_H
