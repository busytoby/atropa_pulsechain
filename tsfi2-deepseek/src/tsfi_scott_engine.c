#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "tsfi_scott_engine.h"
#include "tsfi_lowpower_fet.h"

int tsfi_scott_engine_init(
    uint32_t scott_id,
    tsfi_scott_engine_t *engine
) {
    if (!engine) return -1;
    memset(engine, 0, sizeof(tsfi_scott_engine_t));

    engine->scott_id = scott_id;
    engine->evm_gas_units = 280; // 280 Gas / Auncient Ether Units per evaluation

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    engine->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(engine->tape_dat_bin, sizeof(engine->tape_dat_bin), "SCOTT_DENSITY_%08X.DAT.BIN", scott_id);
    return 0;
}

int tsfi_scott_compute_bandwidth(
    tsfi_scott_engine_t *engine,
    double scale_factor,
    size_t sample_size,
    double *bandwidth_out
) {
    if (!engine || sample_size == 0 || !bandwidth_out) return -1;

    // Non-Gaussian Non-Preferential Accumulator Bandwidth: h = 2.15 * scale_factor * n^(-1/5)
    *bandwidth_out = 2.15 * scale_factor * pow((double)sample_size, -0.2);
    return 0;
}

int tsfi_scott_ash_density_estimate(
    tsfi_scott_engine_t *engine,
    const double *samples,
    size_t n,
    size_t num_bins,
    double *density_bins_out
) {
    if (!engine || !samples || n == 0 || num_bins == 0 || !density_bins_out) return -1;

    // Find min and max of samples
    double min_val = samples[0], max_val = samples[0];
    for (size_t i = 1; i < n; i++) {
        if (samples[i] < min_val) min_val = samples[i];
        if (samples[i] > max_val) max_val = samples[i];
    }

    double range = max_val - min_val;
    if (range == 0.0) range = 1.0;
    double bin_width = range / (double)num_bins;

    memset(density_bins_out, 0, num_bins * sizeof(double));

    // Non-Gaussian biweight accumulator bin weighting across EDO-22 phase space
    for (size_t i = 0; i < n; i++) {
        size_t idx = (size_t)((samples[i] - min_val) / bin_width);
        if (idx >= num_bins) idx = num_bins - 1;
        
        // Non-preferential biweight kernel: (1 - u^2)^2 for |u| <= 1
        double u = (samples[i] - (min_val + (idx + 0.5) * bin_width)) / bin_width;
        if (u < -1.0) u = -1.0;
        if (u > 1.0) u = 1.0;
        double weight = (1.0 - u * u) * (1.0 - u * u);
        
        density_bins_out[idx] += weight;
    }

    // Normalize density
    for (size_t k = 0; k < num_bins; k++) {
        density_bins_out[k] /= ((double)n * bin_width);
    }

    engine->density_evals_count++;
    printf("[D. W. SCOTT NON-GAUSSIAN DENSITY ESTIMATION] ID: %u | Samples: %zu | Bins: %zu | Evals: %u | Gas: %u | File: %s\n",
           engine->scott_id, n, num_bins, engine->density_evals_count, engine->evm_gas_units, engine->tape_dat_bin);

    return 0;
}
