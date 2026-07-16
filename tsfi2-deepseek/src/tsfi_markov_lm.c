#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_markov_lm.h"

void tsfi_markov_lm_init(TSFiMarkovLM *lm) {
    if (!lm) return;
    
    // Set up high probabilities for the sequence "T" -> "O" -> "N" -> "E"
    memset(lm->transition_matrix, 0, sizeof(lm->transition_matrix));

    lm->transition_matrix[PHONEME_T][PHONEME_O] = 0.90f;
    lm->transition_matrix[PHONEME_T][PHONEME_N] = 0.10f;

    lm->transition_matrix[PHONEME_O][PHONEME_N] = 0.95f;
    lm->transition_matrix[PHONEME_O][PHONEME_E] = 0.05f;

    lm->transition_matrix[PHONEME_N][PHONEME_E] = 0.98f;
    lm->transition_matrix[PHONEME_N][PHONEME_T] = 0.02f;

    lm->transition_matrix[PHONEME_E][PHONEME_T] = 0.85f;
    lm->transition_matrix[PHONEME_E][PHONEME_O] = 0.15f;
}

int tsfi_markov_lm_next(TSFiMarkovLM *lm, int current_phoneme, float threshold) {
    if (!lm || current_phoneme < 0 || current_phoneme >= PHONEME_COUNT) return -1;

    // Find transition with highest probability exceeding threshold
    int best_next = -1;
    float max_prob = -1.0f;

    for (int next = 0; next < PHONEME_COUNT; next++) {
        float prob = lm->transition_matrix[current_phoneme][next];
        if (prob > threshold && prob > max_prob) {
            max_prob = prob;
            best_next = next;
        }
    }

    return best_next;
}

int tsfi_markov_lm_step(TSFiMarkovLM *lm, TSFiSynthPerfEngine *engine, TSFiDualStackVM *vm,
                        int *current_phoneme, float threshold) {
    if (!lm || !engine || !vm || !current_phoneme) return -1;

    int next = tsfi_markov_lm_next(lm, *current_phoneme, threshold);

    // CLP Constraint: Verify transition is valid
    if (next == -1) {
        // Fail constraint -> trigger WAM backtrack
        vm->backtrack_triggered = 1;
        return -2;
    }

    // Push new transition to Semantic Stack
    snprintf(vm->semantic_stack[vm->sema_len++], 127, "phoneme/state/%d", next);

    // Update synthesizer frequency based on selected phoneme index
    // PHONEME_T maps to 440Hz, PHONEME_O to 480Hz, etc.
    float target_freq = 440.0f + (float)next * 40.0f;
    engine->synth->wheels[0].frequency = target_freq;

    *current_phoneme = next;
    return 0; // Step completed successfully
}
