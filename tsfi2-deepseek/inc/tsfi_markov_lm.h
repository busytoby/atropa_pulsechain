#ifndef TSFI_MARKOV_LM_H
#define TSFI_MARKOV_LM_H

#include "tsfi_dual_stack.h"
#include "tsfi_synth_perf.h"

// Define phonemes
#define PHONEME_T 0
#define PHONEME_O 1
#define PHONEME_N 2
#define PHONEME_E 3
#define PHONEME_COUNT 4

// Markov Language Model Context
typedef struct {
    float transition_matrix[PHONEME_COUNT][PHONEME_COUNT];
} TSFiMarkovLM;

// Initialize Markov Language Model transition probabilities
void tsfi_markov_lm_init(TSFiMarkovLM *lm);

// Select next phoneme state given current state using transition probabilities
// Returns selected phoneme index, or -1 on constraint failure
int tsfi_markov_lm_next(TSFiMarkovLM *lm, int current_phoneme, float threshold);

// Unified step: Executes transition step, updates synthesizer, and checks VM constraints
int tsfi_markov_lm_step(TSFiMarkovLM *lm, TSFiSynthPerfEngine *engine, TSFiDualStackVM *vm,
                        int *current_phoneme, float threshold);

#endif // TSFI_MARKOV_LM_H
