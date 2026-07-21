#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "tsfi_hershkowitz_fit.h"
#include "tsfi_lowpower_fet.h"

int tsfi_hershkowitz_fit_init(tsfi_hershkowitz_fit_t *fit) {
    if (!fit) return -1;
    memset(fit, 0, sizeof(tsfi_hershkowitz_fit_t));
    fit->evm_gas_units = 320; // 320 Gas / Auncient Ether Units

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    fit->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(fit->tape_dat_bin, sizeof(fit->tape_dat_bin), "HERSHKOWITZ_FIT_00000001.DAT.BIN");
    return 0;
}

int tsfi_hershkowitz_fit_evaluate(
    tsfi_hershkowitz_fit_t *fit,
    const uint32_t *observed,
    const double *expected,
    size_t count
) {
    if (!fit || !observed || !expected || count > HERSHKOWITZ_BINS) return -1;

    double chi_sq = 0.0;
    for (size_t i = 0; i < count; i++) {
        fit->observed_counts[i] = observed[i];
        fit->expected_counts[i] = expected[i];
        if (expected[i] > 0.0) {
            double diff = (double)observed[i] - expected[i];
            chi_sq += (diff * diff) / expected[i];
        }
    }

    fit->chi_square_stat = chi_sq;
    // Critical value for 9 degrees of freedom at alpha=0.05 is 16.919
    fit->is_good_fit = (chi_sq <= 16.919) ? 1 : 0;

    printf("[HERSHKOWITZ FIT] Chi-Square Stat: %.4f | Fit Status: %s | Gas: %u | File: %s\n",
           fit->chi_square_stat, fit->is_good_fit ? "PASS (NORMAL)" : "FAIL (ANOMALY DETECTED)",
           fit->evm_gas_units, fit->tape_dat_bin);

    return 0; // Evaluation successful
}
