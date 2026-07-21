#ifndef TSFI_CHAPPLE_ENGINE_H
#define TSFI_CHAPPLE_ENGINE_H

#include <stdint.h>
#include <stddef.h>

#define MAX_SERIES_DEGREE 16

typedef struct {
    uint32_t chapple_id;
    uint32_t series_inversions_count;   // M. A. Chapple Power Series Reversions
    uint32_t evm_gas_units;             // 280 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_chapple_engine_t;

/* Initialize M. A. Chapple Power Series Inversion Engine */
int tsfi_chapple_engine_init(
    uint32_t chapple_id,
    tsfi_chapple_engine_t *engine
);

/* Compute inverse power series coefficients x = b1*y + b2*y^2 + ... given y = a1*x + a2*x^2 + ... (Lagrange Reversion) */
int tsfi_chapple_revert_power_series(
    tsfi_chapple_engine_t *engine,
    const double *a_coeffs,             // Input coefficients a1, a2, ..., aN (1-indexed at array 0..N-1)
    size_t degree,                       // Degree N (up to MAX_SERIES_DEGREE)
    double *b_coeffs_out                // Output inverse coefficients b1, b2, ..., bN
);

#endif // TSFI_CHAPPLE_ENGINE_H
