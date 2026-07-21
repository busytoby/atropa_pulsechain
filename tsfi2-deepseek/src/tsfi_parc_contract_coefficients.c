#include "tsfi_parc_contract_coefficients.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

int tsfi_contract_coefficients_init(tsfi_contract_coefficient_matrix_t *matrix, const char *dynamic_contract_addr) {
    if (!matrix || !dynamic_contract_addr) return -1;
    memset(matrix, 0, sizeof(tsfi_contract_coefficient_matrix_t));
    matrix->is_initialized = true;

    if (strncmp(dynamic_contract_addr, "dynamic_", 8) == 0) {
        snprintf(matrix->target_contract_address, sizeof(matrix->target_contract_address), "%s", dynamic_contract_addr);
    } else {
        snprintf(matrix->target_contract_address, sizeof(matrix->target_contract_address), "dynamic_%s", dynamic_contract_addr);
    }
    return 0;
}

int tsfi_contract_coefficients_register_holder(tsfi_contract_coefficient_matrix_t *matrix, const char *holder_addr, uint64_t balance) {
    if (!matrix || !holder_addr) return -1;
    if (matrix->holder_count >= TSFI_MAX_HOLDER_COEFFICIENTS) return -2;

    tsfi_holder_coefficient_t *h = &matrix->holders[matrix->holder_count];
    snprintf(h->holder_address, sizeof(h->holder_address), "%s", holder_addr);
    h->balance = balance;
    h->normalized_coefficient = 0.0;

    matrix->holder_count++;
    return 0;
}

int tsfi_contract_coefficients_register_lp(tsfi_contract_coefficient_matrix_t *matrix, const char *pair_addr, uint64_t reserve_a, uint64_t reserve_b) {
    if (!matrix || !pair_addr) return -1;
    if (matrix->lp_count >= TSFI_MAX_LP_COEFFICIENTS) return -2;

    tsfi_lp_coefficient_t *lp = &matrix->lps[matrix->lp_count];
    snprintf(lp->pair_address, sizeof(lp->pair_address), "%s", pair_addr);
    lp->reserve_a = reserve_a;
    lp->reserve_b = reserve_b;
    lp->ratio_coefficient = (reserve_b > 0) ? ((double)reserve_a / (double)reserve_b) : 1.0;

    matrix->lp_count++;
    return 0;
}

int tsfi_contract_coefficients_evaluate(tsfi_contract_coefficient_matrix_t *matrix) {
    if (!matrix || !matrix->is_initialized) return -1;

    uint64_t total_supply = 0;
    for (uint32_t i = 0; i < matrix->holder_count; i++) {
        total_supply += matrix->holders[i].balance;
    }

    double coeff_sum = 0.0;
    for (uint32_t i = 0; i < matrix->holder_count; i++) {
        if (total_supply > 0) {
            matrix->holders[i].normalized_coefficient = (double)matrix->holders[i].balance / (double)total_supply;
        } else {
            matrix->holders[i].normalized_coefficient = 0.0;
        }
        coeff_sum += matrix->holders[i].normalized_coefficient;
    }

    double lp_sum = 0.0;
    for (uint32_t j = 0; j < matrix->lp_count; j++) {
        lp_sum += matrix->lps[j].ratio_coefficient;
    }

    matrix->total_coefficient_sum = coeff_sum + lp_sum;

    // Modulate Lissajous projection frequencies using polynomial contract coefficients
    matrix->dynamic_lissajous_f_x = 1.0 + (coeff_sum * 5.0);
    matrix->dynamic_lissajous_f_y = 1.0 + (lp_sum * 3.0);
    matrix->dynamic_lissajous_f_z = fmod(matrix->total_coefficient_sum * 22.0, 22.0) + 1.0; // EDO-22 pitch mapping

    return 0;
}
