#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_sar_adc.h"

int main() {
    printf("=== TSFi2 & Computer Design Issue #17 Successive Approximation ADC Test ===\n");

    // Test 1: Ideal 8-bit conversion
    printf("[ADC] Testing ideal conversion...\n");
    TsfiSarAdcParams ideal_params = {
        .v_ref = 5.0f,
        .v_offset = 0.0f,
        .v_deadband = 0.0f,
        .dac_tolerances = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
    };

    int cycles = 0;
    uint8_t code_128 = tsfi_sar_adc_convert(2.5f, &ideal_params, &cycles);
    printf("[ADC]   Input: 2.50V -> Code: %d (Expected: 128, Cycles: %d)\n", code_128, cycles);
    assert(code_128 == 128);
    assert(cycles == 8);

    uint8_t code_64 = tsfi_sar_adc_convert(1.25f, &ideal_params, &cycles);
    printf("[ADC]   Input: 1.25V -> Code: %d (Expected: 64)\n", code_64);
    assert(code_64 == 64);

    uint8_t code_192 = tsfi_sar_adc_convert(3.75f, &ideal_params, &cycles);
    printf("[ADC]   Input: 3.75V -> Code: %d (Expected: 192)\n", code_192);
    assert(code_192 == 192);

    // Test 2: ADC with DAC R-2R Resistor Mismatch (INL/DNL errors)
    printf("[ADC] Testing conversion with 6%% MSB R-2R resistor mismatch...\n");
    TsfiSarAdcParams mismatch_params = {
        .v_ref = 5.0f,
        .v_offset = 0.0f,
        .v_deadband = 0.0f,
        .dac_tolerances = {0.06f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f} // +6% error on MSB stage
    };

    // Ideal code for 2.60V is 133
    // With +6% mismatch on MSB, 128 represents 2.65V instead of 2.5V.
    // So 2.60V will fail MSB comparison, resulting in code 127 instead of 133.
    uint8_t code_mismatch = tsfi_sar_adc_convert(2.60f, &mismatch_params, &cycles);
    uint8_t code_ideal = tsfi_sar_adc_convert(2.60f, &ideal_params, &cycles);
    printf("[ADC]   Input: 2.60V -> Ideal Code: %d, Mismatched Code: %d\n", code_ideal, code_mismatch);
    
    assert(code_ideal == 133);
    assert(code_mismatch == 127); // Non-monotonic DNL error demonstrated
    printf("[ADC]   Verification passed: Resistor mismatch successfully simulated DNL/INL jump.\n");

    // Memory sanitation checks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_sar_adc", NULL);

    printf("[PASS] Successive Approximation ADC simulation verified.\n");
    return 0;
}
