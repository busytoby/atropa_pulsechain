#ifndef TSFI_TRANSCENDENTAL_CF_H
#define TSFI_TRANSCENDENTAL_CF_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint64_t numerator_p;   // Best rational convergent numerator p_n
    uint64_t denominator_q; // Best rational convergent denominator q_n
    double approximation;   // Floating point evaluation p_n / q_n
    double error_bound;     // Diophantine error bound |alpha - p/q|
    uint32_t evm_gas_cost;  // EVM Storage Billing (500 Gas Slot)
    char tape_dat_bin[128]; // Rule 13 Filename (.DAT.BIN)
} tsfi_cf_convergent_t;

/* Compute nth convergent for Euler's constant e = [2; 1, 2, 1, 1, 4, 1, 1, 6, ...] */
int tsfi_cf_compute_e_convergent(
    int depth_n,
    tsfi_cf_convergent_t *out_convergent
);

/* Compute nth convergent for Archimedes' constant pi = [3; 7, 15, 1, 292, ...] */
int tsfi_cf_compute_pi_convergent(
    int depth_n,
    tsfi_cf_convergent_t *out_convergent
);

#endif // TSFI_TRANSCENDENTAL_CF_H
