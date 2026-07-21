#ifndef TSFI_PARC_CONTRACT_COEFFICIENTS_H
#define TSFI_PARC_CONTRACT_COEFFICIENTS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TSFI_MAX_HOLDER_COEFFICIENTS 32
#define TSFI_MAX_LP_COEFFICIENTS     16

typedef struct {
    char holder_address[64];
    uint64_t balance;
    double normalized_coefficient; // c_holder,i
} tsfi_holder_coefficient_t;

typedef struct {
    char pair_address[64];
    uint64_t reserve_a;
    uint64_t reserve_b;
    double ratio_coefficient; // c_lp,j
} tsfi_lp_coefficient_t;

typedef struct {
    bool is_initialized;
    char target_contract_address[64]; // dynamic_<address>
    uint32_t holder_count;
    tsfi_holder_coefficient_t holders[TSFI_MAX_HOLDER_COEFFICIENTS];
    uint32_t lp_count;
    tsfi_lp_coefficient_t lps[TSFI_MAX_LP_COEFFICIENTS];
    
    /* Evaluated Contract Polynomial Characteristics */
    double total_coefficient_sum;
    double dynamic_lissajous_f_x;
    double dynamic_lissajous_f_y;
    double dynamic_lissajous_f_z;
} tsfi_contract_coefficient_matrix_t;

/* Initialize Dynamic Contract Coefficient Matrix */
int tsfi_contract_coefficients_init(tsfi_contract_coefficient_matrix_t *matrix, const char *dynamic_contract_addr);

/* Register Holder as Contract Polynomial Coefficient */
int tsfi_contract_coefficients_register_holder(tsfi_contract_coefficient_matrix_t *matrix, const char *holder_addr, uint64_t balance);

/* Register LP Pair as Contract Polynomial Coefficient */
int tsfi_contract_coefficients_register_lp(tsfi_contract_coefficient_matrix_t *matrix, const char *pair_addr, uint64_t reserve_a, uint64_t reserve_b);

/* Evaluate Contract Polynomial & Compute Lissajous Harmonic Frequencies */
int tsfi_contract_coefficients_evaluate(tsfi_contract_coefficient_matrix_t *matrix);

#endif // TSFI_PARC_CONTRACT_COEFFICIENTS_H
