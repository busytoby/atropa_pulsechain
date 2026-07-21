#ifndef TSFI_STEARMAN_ENGINE_H
#define TSFI_STEARMAN_ENGINE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t stearman_id;
    uint32_t samples_processed;
    uint32_t evm_gas_units;             // 280 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_stearman_engine_t;

/* Initialize R. L. Stearman Statistical Subroutine & Resistance Engine */
int tsfi_stearman_engine_init(
    uint32_t stearman_id,
    tsfi_stearman_engine_t *engine
);

/* Compute Sample Statistics & Aerosol Matrix Dispersion */
int tsfi_stearman_compute_stats(
    tsfi_stearman_engine_t *engine,
    const double *data,
    size_t count,
    double *mean_out,
    double *variance_out
);

/* Electrical Resistance Numerical Solver */
int tsfi_stearman_resistance_solve(
    tsfi_stearman_engine_t *engine,
    double current_amps,
    double voltage_volts,
    double *resistance_out
);

#endif // TSFI_STEARMAN_ENGINE_H
