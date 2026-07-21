#ifndef TSFI_SCHMIDT_ENGINE_H
#define TSFI_SCHMIDT_ENGINE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t schmidt_id;
    uint32_t metacompiler_evals;        // Metacompiler Syntax Checks & LU Decompositions
    uint32_t evm_gas_units;             // 280 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_schmidt_engine_t;

/* Initialize Lee Schmidt Syntax Metacompiler & LU Matrix Engine */
int tsfi_schmidt_engine_init(
    uint32_t schmidt_id,
    tsfi_schmidt_engine_t *engine
);

/* Lee Schmidt CACM Syntax Specification Checker for Formal ALGOL Statements */
int tsfi_schmidt_syntax_check(
    tsfi_schmidt_engine_t *engine,
    const char *syntax_expr,
    int *is_valid_out
);

/* Lee Schmidt ALGOL LU Matrix Factorization & Direct Linear System Solver (A = L * U) */
int tsfi_schmidt_lu_factorize(
    tsfi_schmidt_engine_t *engine,
    const double *matrix_A,             // Flattened n x n matrix A
    size_t n,
    double *matrix_L_out,               // Lower triangular n x n matrix L
    double *matrix_U_out                // Upper triangular n x n matrix U
);

#endif // TSFI_SCHMIDT_ENGINE_H
