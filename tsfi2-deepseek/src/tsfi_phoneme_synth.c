#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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

int tsfi_phoneme_apply_jitter_shimmer(float base_freq, float base_amp, int cycle_seed, float *freq_out, float *amp_out) {
    if (!freq_out || !amp_out) return -1;
    
    // Inject micro-jitter: frequency varies by +/- 0.3%
    float jitter_factor = 1.0f + (((float)(cycle_seed % 3) - 1.0f) * 0.003f);
    *freq_out = base_freq * jitter_factor;
    
    // Inject micro-shimmer: amplitude varies by +/- 0.5%
    float shimmer_factor = 1.0f + (((float)(cycle_seed % 5) - 2.0f) * 0.005f);
    *amp_out = base_amp * shimmer_factor;
    
    if (*amp_out > 1.0f) *amp_out = 1.0f;
    if (*amp_out < 0.0f) *amp_out = 0.0f;
    
    return 0;
}

int tsfi_phoneme_apply_vocal_fry(float current_freq, float current_amp, float declination_ratio, float *fry_freq_out, float *fry_amp_out) {
    if (!fry_freq_out || !fry_amp_out) return -1;
    
    *fry_freq_out = current_freq;
    *fry_amp_out = current_amp;
    
    // At the end of utterance (declination_ratio > 0.85), simulate vocal fry drop-off
    if (declination_ratio > 0.85f) {
        *fry_freq_out = 65.0f; // low pitch boundary
        *fry_amp_out = current_amp * 0.4f; // lower volume creakiness
    }
    
    return 0;
}

int tsfi_phoneme_apply_whisper(float base_freq, float base_amp, int is_voiceless, float *whisper_freq_out, float *whisper_amp_out) {
    if (!whisper_freq_out || !whisper_amp_out) return -1;
    
    if (is_voiceless) {
        // Replace base frequency with a high-pitched chaotic frequency to represent whisper noise
        *whisper_freq_out = base_freq * 3.5f;
        // Dampen amplitude to make whisper soft
        *whisper_amp_out = base_amp * 0.35f;
    } else {
        *whisper_freq_out = base_freq;
        *whisper_amp_out = base_amp;
    }
    return 0;
}

int tsfi_phoneme_apply_nasality(float base_freq, int is_nasal, float *nasal_freq_out) {
    if (!nasal_freq_out) return -1;
    
    if (is_nasal) {
        // Attenuate target frequency bounds by applying a notch offset (shifts F0 away from nasal zeros)
        *nasal_freq_out = base_freq * 0.85f;
    } else {
        *nasal_freq_out = base_freq;
    }
    return 0;
}

int tsfi_phoneme_apply_emotion(float base_freq, float base_amp, const char *emotion, float *emo_freq_out, float *emo_amp_out) {
    if (!emotion || !emo_freq_out || !emo_amp_out) return -1;
    
    *emo_freq_out = base_freq;
    *emo_amp_out = base_amp;
    
    if (strcmp(emotion, "angry") == 0) {
        // High frequency (high pitch) and high amplitude (loud/intense)
        *emo_freq_out = base_freq * 1.35f;
        *emo_amp_out = base_amp * 1.4f;
    } else if (strcmp(emotion, "sad") == 0) {
        // Low frequency (low pitch) and dampened amplitude (soft/subdued)
        *emo_freq_out = base_freq * 0.82f;
        *emo_amp_out = base_amp * 0.6f;
    } else if (strcmp(emotion, "happy") == 0) {
        // Higher frequency and slightly boosted amplitude
        *emo_freq_out = base_freq * 1.15f;
        *emo_amp_out = base_amp * 1.2f;
    }
    
    if (*emo_amp_out > 1.0f) *emo_amp_out = 1.0f;
    if (*emo_amp_out < 0.0f) *emo_amp_out = 0.0f;
    
    return 0;
}

int tsfi_phoneme_apply_style_phase(float base_phase, float style_shift, float *aligned_phase_out) {
    if (!aligned_phase_out) return -1;
    
    // Style transfer shifts phase bounds to simulate speaker accent adjustments
    *aligned_phase_out = base_phase + style_shift;
    return 0;
}

int tsfi_phoneme_xiang_map_cantonese_tone(int tone_number, float base_freq, float *tone_freq_out) {
    if (tone_number < 1 || tone_number > 6 || !tone_freq_out) return -1;
    
    switch (tone_number) {
        case 1: *tone_freq_out = base_freq * 1.40f; break; // high flat
        case 2: *tone_freq_out = base_freq * 1.25f; break; // high rising
        case 3: *tone_freq_out = base_freq * 1.10f; break; // mid flat
        case 4: *tone_freq_out = base_freq * 0.80f; break; // low falling
        case 5: *tone_freq_out = base_freq * 0.90f; break; // low rising
        case 6: *tone_freq_out = base_freq * 0.95f; break; // low flat
        default: *tone_freq_out = base_freq; break;
    }
    return 0;
}

int tsfi_phoneme_xiang_detect_liaison(const char *curr_syllable, const char *next_syllable, int *liaison_needed_out) {
    if (!curr_syllable || !next_syllable || !liaison_needed_out) return -1;
    
    *liaison_needed_out = 0;
    
    int curr_len = (int)strlen(curr_syllable);
    if (curr_len > 0) {
        char last_char = curr_syllable[curr_len - 1];
        char first_char = next_syllable[0];
        
        // Vowel to vowel boundary crossing implies liaison transition is active
        int last_vowel = (last_char == 'a' || last_char == 'e' || last_char == 'i' || last_char == 'o' || last_char == 'u');
        int first_vowel = (first_char == 'a' || first_char == 'e' || first_char == 'i' || first_char == 'o' || first_char == 'u');
        
        if (last_vowel && first_vowel) {
            *liaison_needed_out = 1;
        }
    }
    
    return 0;
}

int tsfi_phoneme_xiang_smooth_liaison(float freq_a, float freq_b, float ratio, float *smoothed_freq_out) {
    if (ratio < 0.0f || ratio > 1.0f || !smoothed_freq_out) return -1;
    
    // Linearly interpolate between tone pitch frequencies to smooth syllable transition
    *smoothed_freq_out = (freq_a * (1.0f - ratio)) + (freq_b * ratio);
    return 0;
}

int tsfi_phoneme_xiang_cantonese_sandhi(const int *tones, int count, int target_idx, float base_freq, float *adjusted_freq_out) {
    if (!tones || count <= 0 || target_idx < 0 || target_idx >= count || !adjusted_freq_out) return -1;
    
    *adjusted_freq_out = base_freq;
    
    // Cantonese tone sandhi: consecutive low falling tones (tone 4)
    // If current is tone 4, and next is tone 4, current is shifted to tone 2 (high rising multiplier 1.25)
    if (tones[target_idx] == 4 && target_idx + 1 < count && tones[target_idx + 1] == 4) {
        *adjusted_freq_out = base_freq * 1.5625f; // shifts up (1.25 / 0.8)
    }
    return 0;
}

int tsfi_phoneme_apply_coarticulation(float prev_freq, float curr_freq, float next_freq, float *coart_freq_out) {
    if (!coart_freq_out) return -1;
    
    // Smooth transition by assimilation: blend 70% current, 15% previous, 15% next frequency targets
    *coart_freq_out = (prev_freq * 0.15f) + (curr_freq * 0.70f) + (next_freq * 0.15f);
    return 0;
}

int tsfi_phoneme_generate_glottal_pulse(float time_sample, float pitch_period, float *pulse_out) {
    if (pitch_period <= 0.0f || !pulse_out) return -1;
    
    // Wrap time_sample modulo pitch_period
    float t = time_sample - (pitch_period * (float)((int)(time_sample / pitch_period)));
    if (t < 0.0f) t += pitch_period;
    
    // Rosenberg glottal wave pulse open/close parameters
    float tp = pitch_period * 0.40f; // open time
    float tn = pitch_period * 0.16f; // closing time
    
    if (t < tp) {
        float r = t / tp;
        *pulse_out = 3.0f * r * r - 2.0f * r * r * r;
    } else if (t < tp + tn) {
        float r = (t - tp) / tn;
        *pulse_out = 1.0f - r * r;
    } else {
        *pulse_out = 0.0f;
    }
    return 0;
}

int tsfi_phoneme_xiang_neutralize_sandhi(float speed_ratio, int original_tone, int *neutralized_tone_out) {
    if (!neutralized_tone_out) return -1;
    
    // In rapid speech (>1.4x), Cantonese sandhi neutralizes back to original tone target
    if (speed_ratio > 1.4f) {
        *neutralized_tone_out = original_tone;
    } else {
        *neutralized_tone_out = -1; // keep sandhi active
    }
    return 0;
}

int tsfi_phoneme_xiang_scale_emphasis(const char *particle, int base_duration_ms, int *scaled_duration_out) {
    if (!particle || base_duration_ms <= 0 || !scaled_duration_out) return -1;
    
    *scaled_duration_out = base_duration_ms;
    
    // final particles extension
    if (strcmp(particle, "laa") == 0 || strcmp(particle, "me") == 0 || strcmp(particle, "aa") == 0) {
        *scaled_duration_out = (int)((float)base_duration_ms * 1.5f);
    }
    return 0;
}

int tsfi_phoneme_xiang_compensate_vowel(const char *coda, int base_duration_ms, int *compensated_duration_out) {
    if (!coda || base_duration_ms <= 0 || !compensated_duration_out) return -1;
    
    *compensated_duration_out = base_duration_ms;
    
    // Stop codas shorten vowel duration
    if (strcmp(coda, "p") == 0 || strcmp(coda, "t") == 0 || strcmp(coda, "k") == 0) {
        *compensated_duration_out = (int)((float)base_duration_ms * 0.75f);
    }
    // Nasal codas lengthen vowel duration
    else if (strcmp(coda, "m") == 0 || strcmp(coda, "n") == 0 || strcmp(coda, "ng") == 0) {
        *compensated_duration_out = (int)((float)base_duration_ms * 1.15f);
    }
    return 0;
}

int tsfi_phoneme_xiang_interpolate_pitch_quadratic(float freq_start, float freq_mid, float freq_end, float ratio, float *interpolated_freq_out) {
    if (ratio < 0.0f || ratio > 1.0f || !interpolated_freq_out) return -1;
    
    float inv_r = 1.0f - ratio;
    // P(t) = (1-t)^2 * P0 + 2*(1-t)*t * P1 + t^2 * P2
    *interpolated_freq_out = (inv_r * inv_r * freq_start) + (2.0f * inv_r * ratio * freq_mid) + (ratio * ratio * freq_end);
    return 0;
}

int tsfi_phoneme_xiang_shift_aspect(const char *aspect, float verb_freq, float *aspect_freq_out) {
    if (!aspect || verb_freq <= 0.0f || !aspect_freq_out) return -1;
    
    *aspect_freq_out = verb_freq;
    
    // Zo2 (perfective aspect, high rising) shifts verb frequency up by 25%
    if (strcmp(aspect, "zo2") == 0) {
        *aspect_freq_out = verb_freq * 1.25f;
    }
    // Gan1 (continuous aspect, high flat) shifts verb frequency up by 40%
    else if (strcmp(aspect, "gan1") == 0) {
        *aspect_freq_out = verb_freq * 1.40f;
    }
    return 0;
}

int tsfi_phoneme_smooth_formant(float f_start, float f_end, float ratio, float *smoothed_f_out) {
    if (f_start <= 0.0f || f_end <= 0.0f || ratio < 0.0f || ratio > 1.0f || !smoothed_f_out) return -1;
    
    // Log-linear interpolation
    *smoothed_f_out = expf(logf(f_start) * (1.0f - ratio) + logf(f_end) * ratio);
    return 0;
}

int tsfi_phoneme_feng_adapt_speaker(const float *speaker_embed, int embed_dim, float base_freq, float *adapted_freq_out) {
    if (!speaker_embed || embed_dim <= 0 || base_freq <= 0.0f || !adapted_freq_out) return -1;
    
    float sum = 0.0f;
    for (int i = 0; i < embed_dim; i++) {
        sum += speaker_embed[i];
    }
    float avg = sum / (float)embed_dim;
    
    // Scale baseline pitch relative to average embedding weight bias
    *adapted_freq_out = base_freq * (1.0f + (avg * 0.20f));
    return 0;
}

int tsfi_phoneme_feng_compress_pitch(float current_freq, float median_freq, float compression_factor, float *compressed_freq_out) {
    if (current_freq <= 0.0f || median_freq <= 0.0f || compression_factor < 0.0f || !compressed_freq_out) return -1;
    
    // Compress dynamic range variations around target median pitch
    *compressed_freq_out = median_freq + ((current_freq - median_freq) * compression_factor);
    return 0;
}

int tsfi_phoneme_lin_modulate_stress(int is_stressed, float base_freq, int base_duration_ms, float *stressed_freq_out, int *stressed_duration_out) {
    if (base_freq <= 0.0f || base_duration_ms <= 0 || !stressed_freq_out || !stressed_duration_out) return -1;
    
    if (is_stressed) {
        // Boost pitch frequency and stretch syllable duration
        *stressed_freq_out = base_freq * 1.15f;
        *stressed_duration_out = (int)((float)base_duration_ms * 1.20f);
    } else {
        *stressed_freq_out = base_freq;
        *stressed_duration_out = base_duration_ms;
    }
    return 0;
}

int tsfi_phoneme_lin_adapt_slope(int phrase_syllable_count, float base_slope, float *adapted_slope_out) {
    if (phrase_syllable_count <= 0 || base_slope < 0.0f || !adapted_slope_out) return -1;
    
    // Scale slope scale: longer phrase = flatter slope, shorter phrase = steeper slope
    *adapted_slope_out = base_slope * (1.5f / (1.0f + ((float)phrase_syllable_count * 0.05f)));
    return 0;
}
