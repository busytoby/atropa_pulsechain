#include "tsfi_parc_contract_coefficients.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=======================================================\n");
    printf(" RUNNING MARIARAHEL LAU VIEW-ONLY CONTRACT AUDIT\n");
    printf("=======================================================\n");

    tsfi_contract_coefficient_matrix_t matrix;
    
    // 1. Initialize for mariarahel LAU contract address (0xD32c39fEE49391c7952d1b30b15921b0D3b42E69)
    assert(tsfi_contract_coefficients_init(&matrix, "0xD32c39fEE49391c7952d1b30b15921b0D3b42E69") == 0);
    printf("[PASS] Contract Matrix Initialized for mariarahel LAU: %s\n", matrix.target_contract_address);

    // 2. Setup metadata coefficients (Simulated view outputs from mariarahel contract on PulseChain)
    assert(tsfi_contract_coefficients_set_metadata(&matrix, "mariarahel LAU", "MARIARAHEL", 777000000000000000ULL) == 0);
    printf("[PASS] Configured view-only Metadata Coefficients\n");

    // 3. Register holding interest in AFFECTION (0x24F0154C1dCe548AdF15da2098Fdd8B8A3B8151D)
    assert(tsfi_contract_coefficients_register_holder(&matrix, "0x24F0154C1dCe548AdF15da2098Fdd8B8A3B8151D", 25000000000ULL) == 0);
    printf("[PASS] Registered holding interest in AFFECTION\n");

    // 4. Register Yue function selectors for view-only dynamic query actions
    assert(tsfi_contract_coefficients_register_setting(&matrix, "AFFECTION()", 0x24f0154c) == 0);
    assert(tsfi_contract_coefficients_register_setting(&matrix, "decimals()", 18) == 0);
    printf("[PASS] Registered Yue view-only function selectors\n");

    // 5. Evaluate dynamic spectrum mappings for the TPU quantization coefficients
    assert(tsfi_contract_coefficients_evaluate(&matrix) == 0);
    printf("[PASS] Evaluated dynamic spectrum mapping.\n");
    printf("[INFO] universal_coefficient_sum: %.6f\n", matrix.universal_coefficient_sum);
    printf("[INFO] EDO-22 Pitch Class: %.4f / 22.0\n", matrix.EDO22_pitch_class);
    printf("[INFO] Derived Lissajous Frequencies: f_x=%.2f, f_y=%.2f, f_z=%.2f\n",
           matrix.dynamic_lissajous_f_x, matrix.dynamic_lissajous_f_y, matrix.dynamic_lissajous_f_z);

    // Assert that the derived coefficients are valid within target ranges
    assert(matrix.name_coefficient > 0.0);
    assert(matrix.symbol_coefficient > 0.0);
    assert(matrix.universal_coefficient_sum > 0.0);
    assert(matrix.EDO22_pitch_class >= 0.0 && matrix.EDO22_pitch_class < 22.0);

    printf("=======================================================\n");
    printf(" MARIARAHEL LAU VIEW-ONLY CONTRACT AUDIT PASSED\n");
    printf("=======================================================\n");

    return 0;
}
