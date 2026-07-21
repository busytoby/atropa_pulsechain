#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "tsfi_knuth_fet_gas.h"
#include "tsfi_knuth_storage.h"

int tsfi_knuth_fet_gas_step(
    float initial_vgate,
    float dt,
    tsfi_knuth_fet_gas_state_t *out_state
) {
    if (!out_state || dt <= 0.0f) return -1;
    memset(out_state, 0, sizeof(tsfi_knuth_fet_gas_state_t));

    // 1. Compute FET Discharge Physics via Verlet Solver (Rule 10)
    float decay_rate = 0.5f * 9.81f * dt * dt;
    float v_discharged = (initial_vgate > decay_rate) ? (initial_vgate - decay_rate) : 0.0f;

    out_state->v_gate_charge = initial_vgate;
    out_state->v_gate_discharge_v = v_discharged;

    // 2. Map (Charge, Discharge) to Knuth Base 2i Packed Word
    int32_t x_real = (int32_t)(initial_vgate * 100.0f);
    int32_t y_imag = (int32_t)(v_discharged * 100.0f);
    out_state->packed_base2i = tsfi_knuth_pack_base2i(x_real, y_imag);

    // 3. Compute Net EVM Gas Cost (Base Gas - Refund Delta)
    uint32_t base_gas = 20000;
    uint32_t refund = (uint32_t)((initial_vgate - v_discharged) * 1000.0f);
    if (refund > 4800) refund = 4800; // EVM Refund Cap

    out_state->net_gas_cost = base_gas - refund;

    printf("[KNUTH FET-GAS] FET Charge: %.2fV | Discharged: %.2fV | Base 2i Word: 0x%016lX | Net Gas: %u\n",
           initial_vgate, v_discharged, (unsigned long)out_state->packed_base2i, out_state->net_gas_cost);

    return 0; // Physics and gas step complete
}
