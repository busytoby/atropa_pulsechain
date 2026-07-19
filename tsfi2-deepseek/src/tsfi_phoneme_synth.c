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
    float formant_f1 = 0.0f;
    float formant_f2 = 0.0f;
    int vowel_count = 0;
    int consonant_count = 0;

    // 1. Fully resolve every character of the word to synthesizer parameters
    for (size_t i = 0; i < len; i++) {
        char c = word[i];
        
        // CLP constraint: enforce valid ASCII character bounds
        if (c < 32 || c > 126) return -2; // Backtrack on control characters

        // Detect vowels for Formant Resonance Mapping
        if (c == 'a' || c == 'A') {
            formant_f1 += 730.0f;
            formant_f2 += 1090.0f;
            vowel_count++;
        } else if (c == 'e' || c == 'E') {
            formant_f1 += 530.0f;
            formant_f2 += 1840.0f;
            vowel_count++;
        } else if (c == 'i' || c == 'I') {
            formant_f1 += 270.0f;
            formant_f2 += 2290.0f;
            vowel_count++;
        } else if (c == 'o' || c == 'O') {
            formant_f1 += 570.0f;
            formant_f2 += 840.0f;
            vowel_count++;
        } else if (c == 'u' || c == 'U') {
            formant_f1 += 300.0f;
            formant_f2 += 870.0f;
            vowel_count++;
        } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            // Consonants for noise injection simulation
            consonant_count++;
        }

        // Character value modulates frequency offset (2.0Hz per ASCII step)
        accumulated_freq_offset += (float)c * 2.0f;
        
        // Character position modulates amplitude envelope decay
        total_amplitude += (1.0f / (float)(i + 1));
    }

    // Average amplitude bounds checking [0.0, 1.0]
    total_amplitude = total_amplitude / (float)len;
    if (total_amplitude > 1.0f) total_amplitude = 1.0f;

    // Resolve frequencies: blend with formant frequencies if vowels exist
    float final_freq = base_freq + (accumulated_freq_offset / (float)len);
    if (vowel_count > 0) {
        float avg_f1 = formant_f1 / (float)vowel_count;
        float avg_f2 = formant_f2 / (float)vowel_count;
        // Blend F1/F2 formant center frequencies into the tone wheel frequency
        final_freq = (final_freq + avg_f1 + avg_f2) * 0.5f;
    }

    // Consonantal noise injection simulation: add extremely soft pseudo-random amplitude fluctuation
    if (consonant_count > 0) {
        float noise_jitter = (float)(consonant_count % 7) * 0.005f; // reduced by 10x
        total_amplitude += noise_jitter;
        total_amplitude *= 0.12f; // attenuate overall volume of consonants to keep them soft
        if (total_amplitude > 1.0f) total_amplitude = 1.0f;
        
        // Add a high-frequency jitter component to frequency to simulate noise
        final_freq += (float)(consonant_count % 5) * 5.0f; // reduced jitter range
    }

    // 2. Apply directly to tone-wheel parameters with LGP-30 twin-triode warmth saturation
    // Apply asymmetric quadratic soft-clipping for pleasant second-harmonic tube warmth
    float warmed_amp = total_amplitude + 0.15f * total_amplitude * total_amplitude;
    if (warmed_amp > 1.0f) warmed_amp = 1.0f;

    engine->synth->wheels[wheel_idx].frequency = final_freq;
    engine->synth->wheels[wheel_idx].amplitude = warmed_amp;

    // 3. Write active state change to in-memory AKB rails
    char coord_key[128];
    snprintf(coord_key, sizeof(coord_key), "svdag/phoneme/%d", wheel_idx);
    
    char val_str[64];
    snprintf(val_str, sizeof(val_str), "FREQ_%.2f", engine->synth->wheels[wheel_idx].frequency);
    tsfi_akb_write(engine->synth->orchestrator->akb, coord_key, val_str);

    return 0; // Resolved successfully
}

int tsfi_phoneme_liu_calculate_pause(const int *word_indices, const int *dependency_heads, int count, int target_word_idx, int *pause_ms_out) {
    if (!word_indices || !dependency_heads || count <= 0 || target_word_idx < 0 || target_word_idx >= count || !pause_ms_out) return -1;
    
    // Dependency distance: absolute difference between word index and its head index
    int head_idx = dependency_heads[target_word_idx];
    int distance = 0;
    if (head_idx >= 0 && head_idx < count) {
        distance = abs(target_word_idx - head_idx);
    }
    
    // Pause duration is proportional to dependency distance (e.g. 50ms per unit of distance)
    *pause_ms_out = 50 + (distance * 35);
    return 0;
}

int tsfi_phoneme_liu_adjust_pitch(const int *dependency_heads, int count, int target_word_idx, float base_pitch, float *adjusted_pitch_out) {
    if (!dependency_heads || count <= 0 || target_word_idx < 0 || target_word_idx >= count || !adjusted_pitch_out) return -1;
    
    // Syntactic network node degree centrality: count incoming dependencies + outgoing dependency
    int degree = 0;
    // Outgoing dependency:
    if (dependency_heads[target_word_idx] >= 0 && dependency_heads[target_word_idx] < count) {
        degree++;
    }
    // Incoming dependencies:
    for (int i = 0; i < count; i++) {
        if (dependency_heads[i] == target_word_idx) {
            degree++;
        }
    }
    
    // Adjust pitch higher for keywords with high network degree (central words carry intonation emphasis)
    *adjusted_pitch_out = base_pitch + ((float)degree * 12.5f);
    return 0;
}

int tsfi_phoneme_xu_adjust_sandhi(const int *tones, int count, int target_idx, float base_freq, float *adjusted_freq_out) {
    if (!tones || count <= 0 || target_idx < 0 || target_idx >= count || !adjusted_freq_out) return -1;
    
    *adjusted_freq_out = base_freq;
    
    // Mandarin 3rd-tone sandhi: if current tone is 3, and NEXT tone is also 3, current tone changes to 2nd tone
    if (tones[target_idx] == 3 && target_idx + 1 < count && tones[target_idx + 1] == 3) {
        // Shift frequency higher to represent 2nd tone (rising pitch contour)
        *adjusted_freq_out = base_freq * 1.25f;
    }
    return 0;
}

int tsfi_phoneme_xu_predict_boundary(const char *sentence, int target_char_idx, int *pause_ms_out) {
    if (!sentence || target_char_idx < 0 || target_char_idx >= (int)strlen(sentence) || !pause_ms_out) return -1;
    
    *pause_ms_out = 0;
    char current_char = sentence[target_char_idx];
    
    // Detect phrasing boundary signals from punctuation (e.g. comma, period, or space)
    if (current_char == ',') {
        *pause_ms_out = 250; // short pause for prosodic word/phrase break
    } else if (current_char == '.' || current_char == '?' || current_char == '!') {
        *pause_ms_out = 600; // longer breath pause for sentence boundary
    } else if (current_char == ' ') {
        *pause_ms_out = 80;  // word spacer pause
    }
    
    return 0;
}

int tsfi_phoneme_yu_calculate_declination(int word_progress_idx, int total_words, float base_f0, float *declined_f0_out) {
    if (total_words <= 0 || word_progress_idx < 0 || word_progress_idx >= total_words || !declined_f0_out) return -1;
    
    // Decline slope: drop pitch linearly towards end of sentence (max 15% drop)
    float progress_ratio = (float)word_progress_idx / (float)total_words;
    *declined_f0_out = base_f0 * (1.0f - (progress_ratio * 0.15f));
    return 0;
}

int tsfi_phoneme_yu_estimate_duration(const char *syllable, int *duration_ms_out) {
    if (!syllable || strlen(syllable) == 0 || !duration_ms_out) return -1;
    
    // Base duration for syllable is 150ms
    int duration = 150;
    int len = (int)strlen(syllable);
    
    for (int i = 0; i < len; i++) {
        char c = syllable[i];
        // Vowels extend duration (long open vowels)
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
            c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U') {
            duration += 45;
        } else {
            // Consonants add smaller duration
            duration += 15;
        }
    }
    
    *duration_ms_out = duration;
    return 0;
}

int tsfi_phoneme_reset_sandhi_at_boundary(int pause_duration_ms, int original_tone, int *effective_tone_out) {
    if (!effective_tone_out) return -1;
    
    // If phrasing break is a major breath break (>200ms), reset sandhi modifications (effective tone remains original tone)
    if (pause_duration_ms > 200) {
        *effective_tone_out = original_tone;
    } else {
        // Otherwise, allow tone modifications to propagate (effective tone could be adjusted)
        *effective_tone_out = -1; // flags sandhi propagation is active
    }
    return 0;
}

int tsfi_phoneme_reset_declination_at_boundary(const char *sentence, int char_idx, int current_progress, int *new_progress_out) {
    if (!sentence || char_idx < 0 || char_idx >= (int)strlen(sentence) || !new_progress_out) return -1;
    
    *new_progress_out = current_progress;
    char current_char = sentence[char_idx];
    
    // Reset declination progress to zero when crossing a sentence-level punctuation boundary
    if (current_char == '.' || current_char == '?' || current_char == '!') {
        *new_progress_out = 0;
    }
    return 0;
}
