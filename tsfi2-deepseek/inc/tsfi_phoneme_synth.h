#ifndef TSFI_PHONEME_SYNTH_H
#define TSFI_PHONEME_SYNTH_H

#include "tsfi_synth_perf.h"

// Parse word on a character-by-character level and map directly to tone-wheel parameters
// Returns 0 on success, else negative error code (backtracks on constraint failure)
int tsfi_phoneme_map_word(TSFiSynthPerfEngine *engine, const char *word, int wheel_idx);

// Liu Dependency Distance Pause Optimizer
// Calculates optimal pause duration in milliseconds based on syntactic dependency distances
int tsfi_phoneme_liu_calculate_pause(const int *word_indices, const int *dependency_heads, int count, int target_word_idx, int *pause_ms_out);

// Liu Syntactic Network Intonation Mapper
// Adjusts target pitch offset based on node degree centrality in syntactic dependency network
int tsfi_phoneme_liu_adjust_pitch(const int *dependency_heads, int count, int target_word_idx, float base_pitch, float *adjusted_pitch_out);

// Xu Tone Sandhi Adjuster
// Adjusts Mandarin tones (e.g. 3-3 tone sandhi sequence) and returns modified frequency offset
int tsfi_phoneme_xu_adjust_sandhi(const int *tones, int count, int target_idx, float base_freq, float *adjusted_freq_out);

// Xu Prosodic Boundary Predictor
// Predicts phrasing break pause in milliseconds based on character sequence and punctuation signals
int tsfi_phoneme_xu_predict_boundary(const char *sentence, int target_char_idx, int *pause_ms_out);

// Yu F0 Declination Generator
// Models pitch decay decline slope based on sentence word progress index
int tsfi_phoneme_yu_calculate_declination(int word_progress_idx, int total_words, float base_f0, float *declined_f0_out);

// Yu Syllable Duration Estimator
// Approximates syllable length in milliseconds using phonological vowel/consonant properties
int tsfi_phoneme_yu_estimate_duration(const char *syllable, int *duration_ms_out);

// Sandhi Boundary Resetter
// Resets tone sandhi adjustments if a major phrasing break pause exceeds 200ms
int tsfi_phoneme_reset_sandhi_at_boundary(int pause_duration_ms, int original_tone, int *effective_tone_out);

// Declination Reset Controller
// Resets word progress index to zero if a sentence/prosodic break is detected
int tsfi_phoneme_reset_declination_at_boundary(const char *sentence, int char_idx, int current_progress, int *new_progress_out);

// Jitter and Shimmer Modulator
int tsfi_phoneme_apply_jitter_shimmer(float base_freq, float base_amp, int cycle_seed, float *freq_out, float *amp_out);

// Vocal Fry Region Simulator
int tsfi_phoneme_apply_vocal_fry(float current_freq, float current_amp, float declination_ratio, float *fry_freq_out, float *fry_amp_out);

#endif // TSFI_PHONEME_SYNTH_H
