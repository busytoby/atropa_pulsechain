#include "tsfi_parc_contract_coefficients.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    printf("=======================================================\n");
    printf(" RUNNING DYNAMIC CONTRACT COEFFICIENT AUDIT           \n");
    printf("=======================================================\n");

    tsfi_contract_coefficient_matrix_t matrix;
    assert(tsfi_contract_coefficients_init(&matrix, "0x953467954114363") == 0);
    printf("[PASS] Coefficient Matrix Initialized for dynamic_0x953467954114363\n");

    // Register Holder Coefficients
    assert(tsfi_contract_coefficients_register_holder(&matrix, "0xHolderAlpha", 500000) == 0);
    assert(tsfi_contract_coefficients_register_holder(&matrix, "0xHolderBeta", 500000) == 0);
    printf("[PASS] Registered 2 Holder Balance Coefficients\n");

    // Register LP Pair Coefficients
    assert(tsfi_contract_coefficients_register_lp(&matrix, "0xPairAtropaWpls", 1000000, 2000000) == 0); // ratio 0.5
    printf("[PASS] Registered LP Reserve Ratio Coefficient\n");

    // Evaluate Polynomial & Frequencies
    assert(tsfi_contract_coefficients_evaluate(&matrix) == 0);
    printf("[PASS] Contract Polynomial Evaluated\n");
    printf("[INFO] Total Coefficient Sum: %.4f\n", matrix.total_coefficient_sum);
    printf("[INFO] Derived Lissajous Frequencies: f_x=%.2f, f_y=%.2f, f_z=%.2f\n",
           matrix.dynamic_lissajous_f_x, matrix.dynamic_lissajous_f_y, matrix.dynamic_lissajous_f_z);

    assert(matrix.holder_count == 2);
    assert(matrix.holders[0].normalized_coefficient == 0.5);
    assert(matrix.holders[1].normalized_coefficient == 0.5);
    assert(matrix.lps[0].ratio_coefficient == 0.5);

    printf("=======================================================\n");
    printf(" ALL DYNAMIC CONTRACT COEFFICIENT TESTS PASSED         \n");
    printf("=======================================================\n");

    return 0;
}
