#include "tsfi_terse_fieldata.h"
#include <stdio.h>
#include <string.h>

void fieldata_model_init(FIELDATAProbabilityModel *model) {
    if (!model) return;
    
    // Flat initial distribution: each of the 64 symbols has a frequency of 1
    model->total_freq = FIELDATA_SYMBOLS;
    model->low_cum[0] = 0;
    for (int i = 0; i < FIELDATA_SYMBOLS; i++) {
        model->low_cum[i + 1] = model->low_cum[i] + 1;
    }
}

// Helper to write bits or bytes during arithmetic range coding
int fieldata_terse_compress(
    const uint8_t *input_symbols,
    size_t input_len,
    uint8_t *output_bytes,
    size_t *output_len
) {
    if (!input_symbols || !output_bytes || !output_len) return -1;

    FIELDATAProbabilityModel model;
    fieldata_model_init(&model);

    uint32_t low = 0;
    uint32_t high = 0xFFFFFFFF;
    size_t out_idx = 0;

    for (size_t i = 0; i < input_len; i++) {
        uint8_t sym = input_symbols[i] & 0x3F; // Limit to 6-bit FIELDATA range
        
        uint64_t range = (uint64_t)high - low + 1;
        high = low + (uint32_t)((range * model.low_cum[sym + 1]) / model.total_freq) - 1;
        low = low + (uint32_t)((range * model.low_cum[sym]) / model.total_freq);

        // Shift out matching MSBs
        while ((low ^ high) < 0x01000000) {
            output_bytes[out_idx++] = (uint8_t)(low >> 24);
            low <<= 8;
            high = (high << 8) | 0xFF;
        }
    }

    // Flush remaining bytes
    output_bytes[out_idx++] = (uint8_t)(low >> 24);
    output_bytes[out_idx++] = (uint8_t)(low >> 16);
    output_bytes[out_idx++] = (uint8_t)(low >> 8);
    output_bytes[out_idx++] = (uint8_t)low;

    *output_len = out_idx;
    return 0;
}

int fieldata_terse_decompress(
    const uint8_t *input_bytes,
    size_t input_len,
    uint8_t *output_symbols,
    size_t expected_len
) {
    if (!input_bytes || !output_symbols) return -1;

    FIELDATAProbabilityModel model;
    fieldata_model_init(&model);

    uint32_t low = 0;
    uint32_t high = 0xFFFFFFFF;
    uint32_t value = 0;

    // Load initial 4 bytes of input stream
    size_t in_idx = 0;
    for (int i = 0; i < 4; i++) {
        value = (value << 8) | (in_idx < input_len ? input_bytes[in_idx++] : 0);
    }

    for (size_t i = 0; i < expected_len; i++) {
        uint64_t range = (uint64_t)high - low + 1;
        uint32_t scaled_val = (uint32_t)((((uint64_t)value - low + 1) * model.total_freq - 1) / range);

        // Find corresponding symbol matching cumulative frequency
        uint8_t sym = 0;
        for (int s = 0; s < FIELDATA_SYMBOLS; s++) {
            if (model.low_cum[s + 1] > scaled_val) {
                sym = s;
                break;
            }
        }

        output_symbols[i] = sym;

        high = low + (uint32_t)((range * model.low_cum[sym + 1]) / model.total_freq) - 1;
        low = low + (uint32_t)((range * model.low_cum[sym]) / model.total_freq);

        while ((low ^ high) < 0x01000000) {
            low <<= 8;
            high = (high << 8) | 0xFF;
            value = (value << 8) | (in_idx < input_len ? input_bytes[in_idx++] : 0);
        }
    }

    return 0;
}
