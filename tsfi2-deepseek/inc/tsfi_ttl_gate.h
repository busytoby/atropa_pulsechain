#ifndef TSFI_TTL_GATE_H
#define TSFI_TTL_GATE_H

#include <stdint.h>

typedef struct {
    float v_out;          // Output voltage (Volts)
    int state;            // Logic state (0 or 1)
    float base_charge_pf; // Charge stored in base of phase-splitter transistor
} TsfiTtlNandGate;

void tsfi_ttl_nand_init(TsfiTtlNandGate *gate);
int tsfi_ttl_nand_step(TsfiTtlNandGate *gate, int input_a, int input_b, float dt_ns, float *out_v);

#endif /* TSFI_TTL_GATE_H */
