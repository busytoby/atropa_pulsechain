#ifndef TSFI_SHM_PRUNE_H
#define TSFI_SHM_PRUNE_H

#include "tsfi_akb.h"
#include "tsfi_markov_lm.h"
#include "tsfi_spec_queues.h"

// 1. Shared Memory Handshake Registers Structure
typedef struct {
    volatile int keycode_register;
    volatile int status_register;
} TSFiSHMBridge;

// 2. Markov Nilpotent Pruning Context
typedef struct {
    TSFiMarkovLM *lm;
    float cumulative_threshold;
    TSFiIntrusiveHeap path_heap;
} TSFiPruner;

// Initialize shared-memory registers
void tsfi_shm_bridge_init(TSFiSHMBridge *bridge);

// Write keycode to SHM register (zero-copy)
void tsfi_shm_bridge_write(TSFiSHMBridge *bridge, int keycode);

// Read keycode from SHM register
int tsfi_shm_bridge_read(TSFiSHMBridge *bridge);

int tsfi_shm_prune_path(TSFiPruner *pruner, const int *phoneme_path, int path_len);

int tsfi_shm_prune_push_node(TSFiPruner *pruner, TSFiIntrusiveNode *node);
void tsfi_shm_prune_update_node(TSFiPruner *pruner, TSFiIntrusiveNode *node, int new_priority);
TSFiIntrusiveNode* tsfi_shm_prune_pop_node(TSFiPruner *pruner);

#endif // TSFI_SHM_PRUNE_H
