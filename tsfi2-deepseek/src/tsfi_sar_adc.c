#include "tsfi_sar_adc.h"
#include <string.h>

static float compute_dac_voltage(uint8_t code, const TsfiSarAdcParams *params) {
    float v_dac = 0.0f;
    for (int i = 0; i < 8; i++) {
        int bit = (code >> (7 - i)) & 1;
        if (bit) {
            float fraction = 1.0f / (float)(1 << (i + 1));
            // Apply stage-specific R-2R resistor tolerance mismatch
            v_dac += params->v_ref * fraction * (1.0f + params->dac_tolerances[i]);
        }
    }
    return v_dac;
}

uint8_t tsfi_sar_adc_convert(float v_in, const TsfiSarAdcParams *params, int *num_cycles) {
    uint8_t code = 0;
    int cycles = 0;

    for (int bit = 7; bit >= 0; bit--) {
        cycles++;
        // Set test bit to 1
        code |= (1 << bit);

        // Compute DAC output voltage
        float v_dac = compute_dac_voltage(code, params);

        // Comparator with input offset and deadband
        float diff = v_in - v_dac - params->v_offset;
        int comp_out = 0;

        if (diff >= params->v_deadband / 2.0f) {
            comp_out = 1;
        } else if (diff <= -params->v_deadband / 2.0f) {
            comp_out = 0;
        } else {
            // Indeterminate region: settle to closest direction
            comp_out = (diff >= 0.0f) ? 1 : 0;
        }

        // If DAC voltage is higher than input, clear the test bit
        if (comp_out == 0) {
            code &= ~(1 << bit);
        }
    }

    if (num_cycles) {
        *num_cycles = cycles;
    }
    return code;
}
