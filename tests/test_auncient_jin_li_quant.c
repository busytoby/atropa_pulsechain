#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Simulated Quantization Processor
typedef struct {
    double gamma; // Quantization scale factor (Must be > 0.0)
    gate_state_t quant_gate;
} jin_li_quant_t;

// -------------------------------------------------------------
// Jin Li Texture Quantization Gating
// -------------------------------------------------------------
bool execute_jin_li_quantization(jin_li_quant_t *processor, double raw_value, uint32_t *quantized_out) {
    processor->quant_gate = CUTOFF_STATE;
    *quantized_out = 0;

    // Safety check: Scale factor must be strictly positive to avoid division by zero or inversion
    if (processor->gamma <= 0.0) {
        return false; // Trapped. Quantization cutoff.
    }

    // Gating pass: compute and establish conduction
    processor->quant_gate = CONDUC_STATE;
    *quantized_out = (uint32_t)(raw_value / processor->gamma);
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT JIN LI TEXTURE QUANTIZATION GATING SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    jin_li_quant_t processor = {
        .gamma = 2.5,
        .quant_gate = CUTOFF_STATE
    };

    // 1. Compliant Case: Valid scale factor (2.5) -> Should pass
    printf("[TEST] Checking compliant quantization (gamma = 2.5)...\n");
    fflush(stdout);
    uint32_t out = 0;
    bool ok = execute_jin_li_quantization(&processor, 10.0, &out);
    assert(ok == true);
    assert(processor.quant_gate == CONDUC_STATE);
    assert(out == 4); // 10.0 / 2.5 = 4
    printf("   ✓ Quantization completed. Value: %u\n", out);
    fflush(stdout);

    // 2. Erroneous Case: Invalid scale factor (0.0) -> Should trap
    printf("[TEST] Checking zero scale factor (gamma = 0.0)...\n");
    fflush(stdout);
    processor.gamma = 0.0;
    ok = execute_jin_li_quantization(&processor, 10.0, &out);
    assert(ok == false);
    assert(processor.quant_gate == CUTOFF_STATE);
    printf("   ✓ Division-by-zero hazard trapped successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("JIN LI QUANTIZATION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
