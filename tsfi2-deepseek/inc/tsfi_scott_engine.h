#ifndef TSFI_SCOTT_ENGINE_H
#define TSFI_SCOTT_ENGINE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t scott_id;
    uint32_t density_evals_count;       // Non-Gaussian Non-Preferential Density Evals
    uint32_t evm_gas_units;             // 280 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_scott_engine_t;

/* Initialize D. W. Scott Non-Gaussian Density Estimation Engine */
int tsfi_scott_engine_init(
    uint32_t scott_id,
    tsfi_scott_engine_t *engine
);

/* Compute Scott's Non-Gaussian Uniform/Biweight Optimal Bandwidth: h = 2.15 * scale_factor * n^(-1/5) */
int tsfi_scott_compute_bandwidth(
    tsfi_scott_engine_t *engine,
    double scale_factor,
    size_t sample_size,
    double *bandwidth_out
);

/* Averaged Shifted Histogram (ASH) Non-Gaussian Non-Preferential Accumulator Density Estimation */
int tsfi_scott_ash_density_estimate(
    tsfi_scott_engine_t *engine,
    const double *samples,
    size_t n,
    size_t num_bins,
    double *density_bins_out
);

#endif // TSFI_SCOTT_ENGINE_H
