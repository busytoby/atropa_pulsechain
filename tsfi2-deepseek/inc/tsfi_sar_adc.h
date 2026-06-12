#ifndef TSFI_SAR_ADC_H
#define TSFI_SAR_ADC_H

#include <stdint.h>

typedef struct {
    float v_ref;              // ADC reference voltage (V, typically 5.0V)
    float v_offset;           // Comparator offset voltage (V)
    float v_deadband;         // Comparator sensitivity deadband (V)
    float dac_tolerances[8];  // Resistor mismatch errors for the 8 R-2R stages (percentage fraction)
} TsfiSarAdcParams;

// Runs a full 8-bit Successive Approximation conversion on the input analog voltage.
// v_in: Input analog voltage to convert (0.0 to v_ref)
// params: Converter parameters (DAC errors, comparator offset)
// num_cycles: Output pointer to receive the number of clock cycles taken (exactly 8)
// Returns the resolved 8-bit digital code (0 to 255)
uint8_t tsfi_sar_adc_convert(float v_in, const TsfiSarAdcParams *params, int *num_cycles);

#endif /* TSFI_SAR_ADC_H */
