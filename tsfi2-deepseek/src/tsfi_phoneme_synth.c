#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_phoneme_synth.h"

int tsfi_phoneme_map_word(TSFiSynthPerfEngine *engine, const char *word, int wheel_idx) {
    if (!engine || !word || strlen(word) == 0 || wheel_idx < 0 || wheel_idx >= 4) return -1;

    size_t len = strlen(word);
    float base_freq = engine->synth->wheels[wheel_idx].frequency;
    float accumulated_freq_offset = 0.0f;
    float total_amplitude = 0.0f;

    // 1. Fully resolve every character of the word to synthesizer parameters
    for (size_t i = 0; i < len; i++) {
        char c = word[i];
        
        // CLP constraint: enforce valid ASCII character bounds
        if (c < 32 || c > 126) return -2; // Backtrack on control characters

        // Character value modulates frequency offset (2.0Hz per ASCII step)
        accumulated_freq_offset += (float)c * 2.0f;
        
        // Character position modulates amplitude envelope decay
        total_amplitude += (1.0f / (float)(i + 1));
    }

    // Average amplitude bounds checking [0.0, 1.0]
    total_amplitude = total_amplitude / (float)len;
    if (total_amplitude > 1.0f) total_amplitude = 1.0f;

    // 2. Apply directly to tone-wheel parameters
    engine->synth->wheels[wheel_idx].frequency = base_freq + (accumulated_freq_offset / (float)len);
    engine->synth->wheels[wheel_idx].amplitude = total_amplitude;

    // 3. Write active state change to in-memory AKB rails
    char coord_key[128];
    snprintf(coord_key, sizeof(coord_key), "svdag/phoneme/%d", wheel_idx);
    
    char val_str[64];
    snprintf(val_str, sizeof(val_str), "FREQ_%.2f", engine->synth->wheels[wheel_idx].frequency);
    tsfi_akb_write(engine->synth->orchestrator->akb, coord_key, val_str);

    return 0; // Resolved successfully
}
