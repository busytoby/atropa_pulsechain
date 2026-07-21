#ifndef TSFI_CLENDENIN_TRANSPORT_H
#define TSFI_CLENDENIN_TRANSPORT_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    double junction_temp_celsius; // FET junction temperature in Celsius
    double boundary_flux;        // Clendenin rational boundary transport flux
    double thermal_resistance;   // Effective junction-to-ambient resistance
    uint32_t evm_gas_units;      // 700 Gas / Auncient Ether Units
    double fet_power_watts;      // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];      // Rule 13 Filename (.DAT.BIN)
} tsfi_clendenin_transport_summary_t;

/* Solve Clendenin rational boundary layer transport for FET thermal diffusion */
int tsfi_clendenin_solve_transport(
    double ambient_temp_celsius,
    double energy_injection_joules,
    tsfi_clendenin_transport_summary_t *out_summary
);

#endif // TSFI_CLENDENIN_TRANSPORT_H
