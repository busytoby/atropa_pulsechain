#ifndef TSFI_SHERMAN_ENGINE_H
#define TSFI_SHERMAN_ENGINE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t sherman_id;
    uint32_t matrix_updates_count;      // Sherman-Morrison Rank-1 Inverse Updates
    uint32_t evm_gas_units;             // 280 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_sherman_engine_t;

/* Initialize Jack Sherman Rank-1 Matrix Inverse Update Engine */
int tsfi_sherman_engine_init(
    uint32_t sherman_id,
    tsfi_sherman_engine_t *engine
);

/* Sherman-Morrison Rank-1 Update Solver: (A + u v^T)^-1 = A^-1 - (A^-1 u v^T A^-1)/(1 + v^T A^-1 u) */
int tsfi_sherman_morrison_update(
    tsfi_sherman_engine_t *engine,
    const double *inv_A,                // Flattened n x n inverse matrix A^-1
    const double *u,                    // n-dim column vector
    const double *v,                    // n-dim column vector
    size_t n,
    double *inv_updated_out             // Flattened n x n updated inverse matrix
);

#endif // TSFI_SHERMAN_ENGINE_H
