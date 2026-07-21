#ifndef TSFI_SCOTT_ENGINE_H
#define TSFI_SCOTT_ENGINE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t scott_id;
    uint32_t density_evals_count;       // Scott's Rule Bandwidth & ASH Density Evals
    uint32_t evm_gas_units;             // 280 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_scott_engine_t;

/* Initialize D. W. Scott Density Estimation Engine */
int tsfi_scott_engine_init(
    uint32_t scott_id,
    tsfi_scott_engine_t *engine
);

/* Compute Scott's Rule Optimal Bandwidth: h = 3.49 * std_dev * n^(-1/3) */
int tsfi_scott_compute_bandwidth(
    tsfi_scott_engine_t *engine,
    double std_dev,
    size_t sample_size,
    double *bandwidth_out
);

/* Averaged Shifted Histogram (ASH) Nonparametric Density Estimation */
int tsfi_scott_ash_density_estimate(
    tsfi_scott_engine_t *engine,
    const double *samples,
    size_t n,
    size_t num_bins,
    double *density_bins_out
);

#endif // TSFI_SCOTT_ENGINE_H
