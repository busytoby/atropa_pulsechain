#include "tsfi_parc_contract_coefficients.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    printf("=======================================================\n");
    printf(" RUNNING UNIVERSAL CONTRACT COEFFICIENT AUDIT         \n");
    printf("=======================================================\n");

    tsfi_contract_coefficient_matrix_t matrix;
    assert(tsfi_contract_coefficients_init(&matrix, "0x953467954114363") == 0);
    printf("[PASS] Universal Matrix Initialized for dynamic_0x953467954114363\n");

    // 1. Metadata Coefficients (Name, Symbol, Total Supply)
    assert(tsfi_contract_coefficients_set_metadata(&matrix, "Atropa PulseChain", "ATROPA", 1000000000000000000ULL) == 0);
    printf("[PASS] Set Metadata Coefficients (Name, Symbol, TotalSupply)\n");
    printf("[INFO] Name Coefficient c_name: %.6f\n", matrix.name_coefficient);
    printf("[INFO] Symbol Coefficient c_symbol: %.6f\n", matrix.symbol_coefficient);
    printf("[INFO] TotalSupply Coefficient c_totalSupply: %.6f\n", matrix.total_supply_coefficient);

    // 2. Settings & Function Selector Coefficients
    assert(tsfi_contract_coefficients_register_setting(&matrix, "owner()", 0x12345678) == 0);
    assert(tsfi_contract_coefficients_register_setting(&matrix, "taxFee()", 2) == 0);
    assert(tsfi_contract_coefficients_register_setting(&matrix, "isPaused()", 0) == 0);
    printf("[PASS] Registered 3 Function/Setting Coefficients\n");

    // 3. Holder Coefficients
    assert(tsfi_contract_coefficients_register_holder(&matrix, "0xHolderAlpha", 500000) == 0);
    assert(tsfi_contract_coefficients_register_holder(&matrix, "0xHolderBeta", 500000) == 0);
    printf("[PASS] Registered 2 Holder Balance Coefficients\n");

    // 4. LP Pair Coefficients
    assert(tsfi_contract_coefficients_register_lp(&matrix, "0xPairAtropaWpls", 1000000, 2000000) == 0); // ratio 0.5
    printf("[PASS] Registered LP Reserve Ratio Coefficient\n");

    // 5. Evaluate Spectrum
    assert(tsfi_contract_coefficients_evaluate(&matrix) == 0);
    printf("[PASS] Universal Contract Polynomial Evaluated Spectrum\n");
    printf("[INFO] Universal Coefficient Sum: %.6f\n", matrix.universal_coefficient_sum);
    printf("[INFO] EDO-22 Pitch Class: %.4f / 22.0\n", matrix.EDO22_pitch_class);
    printf("[INFO] Derived Lissajous Frequencies: f_x=%.2f, f_y=%.2f, f_z=%.2f\n",
           matrix.dynamic_lissajous_f_x, matrix.dynamic_lissajous_f_y, matrix.dynamic_lissajous_f_z);

    assert(matrix.name_coefficient > 0.0);
    assert(matrix.symbol_coefficient > 0.0);
    assert(matrix.total_supply_coefficient > 0.0);
    assert(matrix.setting_count == 3);

    printf("=======================================================\n");
    printf(" ALL UNIVERSAL CONTRACT COEFFICIENT TESTS PASSED       \n");
    printf("=======================================================\n");

    return 0;
}
