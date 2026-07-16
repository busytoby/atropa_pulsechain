#ifndef TSFI_PHONEME_SYNTH_H
#define TSFI_PHONEME_SYNTH_H

#include "tsfi_synth_perf.h"

// Parse word on a character-by-character level and map directly to tone-wheel parameters
// Returns 0 on success, else negative error code (backtracks on constraint failure)
int tsfi_phoneme_map_word(TSFiSynthPerfEngine *engine, const char *word, int wheel_idx);

#endif // TSFI_PHONEME_SYNTH_H
