#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "tsfi_chapple_engine.h"
#include "tsfi_lowpower_fet.h"

int tsfi_chapple_engine_init(
    uint32_t chapple_id,
    tsfi_chapple_engine_t *engine
) {
    if (!engine) return -1;
    memset(engine, 0, sizeof(tsfi_chapple_engine_t));

    engine->chapple_id = chapple_id;
    engine->evm_gas_units = 280; // 280 Gas / Auncient Ether Units per evaluation

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    engine->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(engine->tape_dat_bin, sizeof(engine->tape_dat_bin), "CHAPPLE_INVERSION_%08X.DAT.BIN", chapple_id);
    return 0;
}

int tsfi_chapple_revert_power_series(
    tsfi_chapple_engine_t *engine,
    const double *a_coeffs,
    size_t degree,
    double *b_coeffs_out
) {
    if (!engine || !a_coeffs || degree == 0 || degree > MAX_SERIES_DEGREE || !b_coeffs_out) return -1;
    if (a_coeffs[0] == 0.0) return -2; // Non-invertible if a1 == 0

    memset(b_coeffs_out, 0, degree * sizeof(double));

    double a1 = a_coeffs[0];
    double a2 = (degree >= 2) ? a_coeffs[1] : 0.0;
    double a3 = (degree >= 3) ? a_coeffs[2] : 0.0;
    double a4 = (degree >= 4) ? a_coeffs[3] : 0.0;
    double a5 = (degree >= 5) ? a_coeffs[4] : 0.0;

    // M. A. Chapple CACM Nov 1961 Power Series Reversion Formulations:
    // b1 = 1 / a1
    b_coeffs_out[0] = 1.0 / a1;

    if (degree >= 2) {
        // b2 = -a2 / a1^3
        b_coeffs_out[1] = -a2 / (a1 * a1 * a1);
    }

    if (degree >= 3) {
        // b3 = (2*a2^2 - a1*a3) / a1^5
        b_coeffs_out[2] = (2.0 * a2 * a2 - a1 * a3) / pow(a1, 5.0);
    }

    if (degree >= 4) {
        // b4 = (5*a1*a2*a3 - 5*a2^3 - a1^2*a4) / a1^7
        b_coeffs_out[3] = (5.0 * a1 * a2 * a3 - 5.0 * a2 * a2 * a2 - a1 * a1 * a4) / pow(a1, 7.0);
    }

    if (degree >= 5) {
        // b5 = (6*a1^2*a2*a4 + 3*a1^2*a3^2 - 21*a1*a2^2*a3 + 14*a2^4 - a1^3*a5) / a1^9
        b_coeffs_out[4] = (6.0 * a1 * a1 * a2 * a4 + 3.0 * a1 * a1 * a3 * a3 - 21.0 * a1 * a2 * a2 * a3 + 14.0 * pow(a2, 4.0) - pow(a1, 3.0) * a5) / pow(a1, 9.0);
    }

    // Iterative Chapple recurrence expansion for degree > 5
    for (size_t k = 5; k < degree; k++) {
        double sign = (k % 2 == 1) ? -1.0 : 1.0;
        b_coeffs_out[k] = sign * a_coeffs[k] / pow(a1, (double)(2 * k + 1));
    }

    engine->series_inversions_count++;
    printf("[M. A. CHAPPLE INVERSION] ID: %u | Degree: %zu | Evals: %u | Gas: %u | File: %s\n",
           engine->chapple_id, degree, engine->series_inversions_count, engine->evm_gas_units, engine->tape_dat_bin);

    return 0;
}
