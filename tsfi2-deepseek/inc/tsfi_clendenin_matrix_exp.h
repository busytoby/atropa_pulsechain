#ifndef TSFI_CLENDENIN_MATRIX_EXP_H
#define TSFI_CLENDENIN_MATRIX_EXP_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    double matrix_out[16];      // 4x4 Result matrix e^(A*t)
    double matrix_trace;        // Trace of resulting state matrix
    double matrix_determinant;  // Determinant of resulting matrix
    uint32_t evm_gas_units;     // 750 Gas / Auncient Ether Units
    double fet_power_watts;     // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];     // Rule 13 Filename (.DAT.BIN)
} tsfi_clendenin_matrix_summary_t;

/* Compute 4x4 matrix exponential e^(A*t) using Clendenin rational Padé approximation */
int tsfi_clendenin_matrix_exp(
    const double matrix_a[16],
    double t,
    tsfi_clendenin_matrix_summary_t *out_summary
);

#endif // TSFI_CLENDENIN_MATRIX_EXP_H
