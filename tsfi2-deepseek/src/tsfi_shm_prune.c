#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_shm_prune.h"

void tsfi_shm_bridge_init(TSFiSHMBridge *bridge) {
    if (!bridge) return;
    bridge->keycode_register = 0;
    bridge->status_register = 0;
}

void tsfi_shm_bridge_write(TSFiSHMBridge *bridge, int keycode) {
    if (!bridge) return;
    bridge->keycode_register = keycode;
    bridge->status_register = 1; // Mark buffer as ready
}

int tsfi_shm_bridge_read(TSFiSHMBridge *bridge) {
    if (!bridge || bridge->status_register == 0) return -1;
    int val = bridge->keycode_register;
    bridge->status_register = 0; // Clear read status
    return val;
}

int tsfi_shm_prune_path(TSFiPruner *pruner, const int *phoneme_path, int path_len) {
    if (!pruner || !pruner->lm || !phoneme_path || path_len <= 1) return 0;

    float current_probability = 1.0f;

    for (int i = 0; i < path_len - 1; i++) {
        int src = phoneme_path[i];
        int dest = phoneme_path[i + 1];

        if (src < 0 || src >= PHONEME_COUNT || dest < 0 || dest >= PHONEME_COUNT) {
            return 1; // Pruned immediately (Invalid index)
        }

        // Multiply transition probability (Chain Rule of Probability)
        float transition_prob = pruner->lm->transition_matrix[src][dest];
        current_probability *= transition_prob;

        // Pruning constraint: Abort evaluation if probability drops below threshold (Nilpotent prune)
        if (current_probability < pruner->cumulative_threshold) {
            return 1; // Pruned
        }
    }

    return 0; // Allowed
}
