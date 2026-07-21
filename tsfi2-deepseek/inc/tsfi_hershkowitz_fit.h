#ifndef TSFI_HERSHKOWITZ_FIT_H
#define TSFI_HERSHKOWITZ_FIT_H

#include <stdint.h>
#include <stddef.h>

#define HERSHKOWITZ_BINS 10

typedef struct {
    uint32_t observed_counts[HERSHKOWITZ_BINS]; // Observed transaction counts per bin
    double expected_counts[HERSHKOWITZ_BINS];   // Expected theoretical counts
    double chi_square_stat;                    // Calculated Chi-Square statistic
    uint8_t is_good_fit;                       // 1 = Pass Goodness of Fit, 0 = Anomaly
    uint32_t evm_gas_units;                    // 320 Gas / Auncient Ether Units
    double fet_power_watts;                    // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];                    // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_hershkowitz_fit_t;

/* Initialize a Hershkowitz Goodness-of-Fit Statistical Evaluator */
int tsfi_hershkowitz_fit_init(tsfi_hershkowitz_fit_t *fit);

/* Compute Chi-Square Goodness of Fit test on ledger transaction distributions */
int tsfi_hershkowitz_fit_evaluate(
    tsfi_hershkowitz_fit_t *fit,
    const uint32_t *observed,
    const double *expected,
    size_t count
);

#endif // TSFI_HERSHKOWITZ_FIT_H
