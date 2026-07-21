#ifndef TSFI_THACHER_SOLVER_H
#define TSFI_THACHER_SOLVER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t solver_id;
    uint32_t evaluations_count;
    uint32_t evm_gas_units;             // 280 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_thacher_solver_t;

/* Initialize H. C. Thacher Rational Approximation & ALGOL Suite */
int tsfi_thacher_solver_init(
    uint32_t solver_id,
    tsfi_thacher_solver_t *solver
);

/* Thacher Rational Chebyshev Approximation Solver R(x) = P(x) / Q(x) */
int tsfi_thacher_rational_eval(
    tsfi_thacher_solver_t *solver,
    double x,
    double *result_out
);

/* CACM Certified ALGOL 60 Polynomial System Integration */
int tsfi_thacher_cacm_algol_quadrature(
    tsfi_thacher_solver_t *solver,
    double a,
    double b,
    double *integral_out
);

#endif // TSFI_THACHER_SOLVER_H
