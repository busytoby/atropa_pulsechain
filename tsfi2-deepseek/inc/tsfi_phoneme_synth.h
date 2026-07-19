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

// Whispery Voice Simulator
int tsfi_phoneme_apply_whisper(float base_freq, float base_amp, int is_voiceless, float *whisper_freq_out, float *whisper_amp_out);

// Nasality Resonance Simulator
int tsfi_phoneme_apply_nasality(float base_freq, int is_nasal, float *nasal_freq_out);

// Sun Emotional Prosody Scaler
int tsfi_phoneme_apply_emotion(float base_freq, float base_amp, const char *emotion, float *emo_freq_out, float *emo_amp_out);

// Sun Style Phase Aligner
int tsfi_phoneme_apply_style_phase(float base_phase, float style_shift, float *aligned_phase_out);

// Xiang Cantonese 6-Tone Mapper
int tsfi_phoneme_xiang_map_cantonese_tone(int tone_number, float base_freq, float *tone_freq_out);

// Xiang Syllable Liaison Detector
int tsfi_phoneme_xiang_detect_liaison(const char *curr_syllable, const char *next_syllable, int *liaison_needed_out);

// Xiang Liaison Transition Smoother
int tsfi_phoneme_xiang_smooth_liaison(float freq_a, float freq_b, float ratio, float *smoothed_freq_out);

// Xiang Cantonese Tone Sandhi Controller
int tsfi_phoneme_xiang_cantonese_sandhi(const int *tones, int count, int target_idx, float base_freq, float *adjusted_freq_out);

// Tone Co-articulation Modulator
int tsfi_phoneme_apply_coarticulation(float prev_freq, float curr_freq, float next_freq, float *coart_freq_out);

// Glottal Flow Excitation Generator
int tsfi_phoneme_generate_glottal_pulse(float time_sample, float pitch_period, float *pulse_out);

// Xiang Cantonese Sandhi Neutralizer
int tsfi_phoneme_xiang_neutralize_sandhi(float speed_ratio, int original_tone, int *neutralized_tone_out);

// Xiang Syllable Emphasis Scaler
int tsfi_phoneme_xiang_scale_emphasis(const char *particle, int base_duration_ms, int *scaled_duration_out);

#endif // TSFI_PHONEME_SYNTH_H
