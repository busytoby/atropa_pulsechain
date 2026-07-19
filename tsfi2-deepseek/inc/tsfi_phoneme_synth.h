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

#endif // TSFI_PHONEME_SYNTH_H
