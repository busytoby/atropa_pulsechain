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

void fieldata_model_update(FIELDATAProbabilityModel *model, uint8_t sym) {
    if (!model) return;

    // Increment frequencies of all values after sym to shift cumulative bands
    for (int s = sym + 1; s <= FIELDATA_SYMBOLS; s++) {
        model->low_cum[s]++;
    }
    model->total_freq++;

    // Halve frequencies if total accumulates past limit to prevent overflow & maintain adaptivity
    if (model->total_freq >= 16384) {
        uint32_t current_cum = 0;
        for (int s = 0; s < FIELDATA_SYMBOLS; s++) {
            uint32_t freq = model->low_cum[s + 1] - model->low_cum[s];
            freq = (freq >> 1) | 1; // Halve, preserving minimum frequency of 1
            model->low_cum[s] = current_cum;
            current_cum += freq;
        }
        model->low_cum[FIELDATA_SYMBOLS] = current_cum;
        model->total_freq = current_cum;
    }
}

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

        // Update probability model dynamically
        fieldata_model_update(&model, sym);

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

        // Binary search corresponding symbol matching cumulative frequency
        uint8_t sym = 0;
        int low_idx = 0;
        int high_idx = FIELDATA_SYMBOLS - 1;
        while (low_idx <= high_idx) {
            int mid = (low_idx + high_idx) / 2;
            if (model.low_cum[mid] <= scaled_val && model.low_cum[mid + 1] > scaled_val) {
                sym = mid;
                break;
            }
            if (model.low_cum[mid] > scaled_val) {
                high_idx = mid - 1;
            } else {
                low_idx = mid + 1;
            }
        }

        output_symbols[i] = sym;

        high = low + (uint32_t)((range * model.low_cum[sym + 1]) / model.total_freq) - 1;
        low = low + (uint32_t)((range * model.low_cum[sym]) / model.total_freq);

        // Update probability model dynamically in lockstep with compressor
        fieldata_model_update(&model, sym);

        while ((low ^ high) < 0x01000000) {
            low <<= 8;
            high = (high << 8) | 0xFF;
            value = (value << 8) | (in_idx < input_len ? input_bytes[in_idx++] : 0);
        }
    }

    return 0;
}

void fieldata_pack_36bit(const uint8_t *symbols, size_t sym_len, uint64_t *words, size_t *word_count) {
    if (!symbols || !words || !word_count) return;
    
    size_t count = 0;
    uint64_t current_word = 0;
    int bit_offset = 30; // Packing six 6-bit symbols: bits 30, 24, 18, 12, 6, 0
    
    for (size_t i = 0; i < sym_len; i++) {
        uint64_t sym_val = symbols[i] & 0x3F;
        current_word |= (sym_val << bit_offset);
        bit_offset -= 6;
        
        if (bit_offset < 0) {
            words[count++] = current_word;
            current_word = 0;
            bit_offset = 30;
        }
    }
    
    if (bit_offset < 30) {
        words[count++] = current_word; // Flush partially packed word
    }
    *word_count = count;
}

void fieldata_unpack_36bit(const uint64_t *words, size_t word_count, uint8_t *symbols, size_t *sym_len) {
    if (!words || !symbols || !sym_len) return;
    
    size_t count = 0;
    for (size_t i = 0; i < word_count; i++) {
        uint64_t word = words[i];
        int bit_offset = 30;
        while (bit_offset >= 0) {
            symbols[count++] = (uint8_t)((word >> bit_offset) & 0x3F);
            bit_offset -= 6;
        }
    }
    *sym_len = count;
}

void fieldata_utf6_encode(uint32_t codepoint, uint8_t *out_symbols, size_t *out_len) {
    if (!out_symbols || !out_len) return;

    if (codepoint <= 60) {
        out_symbols[0] = (uint8_t)codepoint;
        *out_len = 1;
    } else if (codepoint <= 0x3FFFF) {
        out_symbols[0] = 61; // Shift-18 indicator
        out_symbols[1] = (uint8_t)((codepoint >> 12) & 0x3F);
        out_symbols[2] = (uint8_t)((codepoint >> 6) & 0x3F);
        out_symbols[3] = (uint8_t)(codepoint & 0x3F);
        *out_len = 4;
    } else if (codepoint <= 0xFFFFFF) {
        out_symbols[0] = 62; // Shift-24 indicator
        out_symbols[1] = (uint8_t)((codepoint >> 18) & 0x3F);
        out_symbols[2] = (uint8_t)((codepoint >> 12) & 0x3F);
        out_symbols[3] = (uint8_t)((codepoint >> 6) & 0x3F);
        out_symbols[4] = (uint8_t)(codepoint & 0x3F);
        *out_len = 5;
    } else {
        out_symbols[0] = 63; // Shift-36 indicator (holds full 32-bit codepoint)
        out_symbols[1] = (uint8_t)((codepoint >> 30) & 0x3F);
        out_symbols[2] = (uint8_t)((codepoint >> 24) & 0x3F);
        out_symbols[3] = (uint8_t)((codepoint >> 18) & 0x3F);
        out_symbols[4] = (uint8_t)((codepoint >> 12) & 0x3F);
        out_symbols[5] = (uint8_t)((codepoint >> 6) & 0x3F);
        out_symbols[6] = (uint8_t)(codepoint & 0x3F);
        *out_len = 7;
    }
}

uint32_t fieldata_utf6_decode(const uint8_t *symbols, size_t *in_len) {
    if (!symbols || !in_len) return 0;

    uint8_t lead = symbols[0];
    if (lead <= 60) {
        *in_len = 1;
        return lead;
    } else if (lead == 61) {
        *in_len = 4;
        return ((uint32_t)symbols[1] << 12) | ((uint32_t)symbols[2] << 6) | (symbols[3]);
    } else if (lead == 62) {
        *in_len = 5;
        return ((uint32_t)symbols[1] << 18) | ((uint32_t)symbols[2] << 12) | ((uint32_t)symbols[3] << 6) | (symbols[4]);
    } else if (lead == 63) {
        *in_len = 7;
        return ((uint32_t)symbols[1] << 30) | ((uint32_t)symbols[2] << 24) | ((uint32_t)symbols[3] << 18) |
               ((uint32_t)symbols[4] << 12) | ((uint32_t)symbols[5] << 6) | (symbols[6]);
    }
    return 0;
}
