#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_sherman_engine.h"
#include "tsfi_lowpower_fet.h"

int tsfi_sherman_engine_init(
    uint32_t sherman_id,
    tsfi_sherman_engine_t *engine
) {
    if (!engine) return -1;
    memset(engine, 0, sizeof(tsfi_sherman_engine_t));

    engine->sherman_id = sherman_id;
    engine->evm_gas_units = 280; // 280 Gas / Auncient Ether Units per evaluation

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    engine->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(engine->tape_dat_bin, sizeof(engine->tape_dat_bin), "SHERMAN_UPDATE_%08X.DAT.BIN", sherman_id);
    return 0;
}

int tsfi_sherman_morrison_update(
    tsfi_sherman_engine_t *engine,
    const double *inv_A,
    const double *u,
    const double *v,
    size_t n,
    double *inv_updated_out
) {
    if (!engine || !inv_A || !u || !v || n == 0 || !inv_updated_out) return -1;

    // 1. Compute A^-1 * u -> vector w (n elements)
    double *w = (double *)calloc(n, sizeof(double));
    if (!w) return -1;

    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            w[i] += inv_A[i * n + j] * u[j];
        }
    }

    // 2. Compute v^T * A^-1 -> vector z^T (n elements) where z_j = sum_i (v_i * inv_A[i][j])
    double *z = (double *)calloc(n, sizeof(double));
    if (!z) { free(w); return -1; }

    for (size_t j = 0; j < n; j++) {
        for (size_t i = 0; i < n; i++) {
            z[j] += v[i] * inv_A[i * n + j];
        }
    }

    // 3. Compute scalar alpha = 1 + v^T * A^-1 * u = 1 + sum_i (v_i * w_i)
    double v_w_dot = 0.0;
    for (size_t i = 0; i < n; i++) {
        v_w_dot += v[i] * w[i];
    }
    double alpha = 1.0 + v_w_dot;
    if (alpha == 0.0) {
        free(w); free(z);
        return -2; // Singular rank-1 perturbation
    }

    // 4. Update matrix: inv_updated[i][j] = inv_A[i][j] - (w[i] * z[j]) / alpha
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            inv_updated_out[i * n + j] = inv_A[i * n + j] - (w[i] * z[j]) / alpha;
        }
    }

    free(w);
    free(z);

    engine->matrix_updates_count++;
    printf("[JACK SHERMAN RANK-1 UPDATE] ID: %u | Matrix Sz: %zux%zu | Updates: %u | Gas: %u | File: %s\n",
           engine->sherman_id, n, n, engine->matrix_updates_count, engine->evm_gas_units, engine->tape_dat_bin);

    return 0;
}
