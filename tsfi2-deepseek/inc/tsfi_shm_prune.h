#ifndef TSFI_SHM_PRUNE_H
#define TSFI_SHM_PRUNE_H

#include "tsfi_akb.h"
#include "tsfi_markov_lm.h"

// 1. Shared Memory Handshake Registers Structure
typedef struct {
    volatile int keycode_register;
    volatile int status_register;
} TSFiSHMBridge;

// 2. Markov Nilpotent Pruning Context
typedef struct {
    TSFiMarkovLM *lm;
    float cumulative_threshold;
} TSFiPruner;

// Initialize shared-memory registers
void tsfi_shm_bridge_init(TSFiSHMBridge *bridge);

// Write keycode to SHM register (zero-copy)
void tsfi_shm_bridge_write(TSFiSHMBridge *bridge, int keycode);

// Read keycode from SHM register
int tsfi_shm_bridge_read(TSFiSHMBridge *bridge);

// Evaluates SVDAG path transitions, pruning early if cumulative probability is too low
// Returns 0 if path is allowed, 1 if path is pruned
int tsfi_shm_prune_path(TSFiPruner *pruner, const int *phoneme_path, int path_len);

#endif // TSFI_SHM_PRUNE_H
