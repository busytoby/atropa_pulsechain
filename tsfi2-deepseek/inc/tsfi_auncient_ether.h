#ifndef TSFI_AUNCIENT_ETHER_H
#define TSFI_AUNCIENT_ETHER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint64_t evm_gas_units;      // Billed EVM Gas units
    double ether_energy_joules;  // Equivalent Auncient Ether energy in Joules
    double fet_power_watts;      // FET discharge power (0.0109 W under 78.2% Cut)
    double v_gate_discharge;     // Verlet soft-body gate discharge voltage (3.3V)
    char tape_dat_bin[128];      // Rule 13 Filename (.DAT.BIN)
} tsfi_auncient_ether_summary_t;

/* Convert EVM Gas units into physical Auncient Ether discharge metrics */
int tsfi_auncient_ether_meter_discharge(
    uint64_t evm_gas_units,
    tsfi_auncient_ether_summary_t *out_summary
);

#endif // TSFI_AUNCIENT_ETHER_H
