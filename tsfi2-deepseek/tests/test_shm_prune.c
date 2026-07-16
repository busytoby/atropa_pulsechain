#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_shm_prune.h"

int main(void) {
    printf("[SHM & Pruning] Starting zero-copy register and graph pruning tests...\n");
    fflush(stdout);

    // 1. Verify Shared Memory Handshake
    TSFiSHMBridge bridge;
    tsfi_shm_bridge_init(&bridge);

    tsfi_shm_bridge_write(&bridge, 32); // Write 'D' Keycode
    int val = tsfi_shm_bridge_read(&bridge);
    printf("  [SHM Handshake] Read Value: %d (Expected 32)\n", val);
    fflush(stdout);
    assert(val == 32);

    // Clear read register returns -1
    val = tsfi_shm_bridge_read(&bridge);
    assert(val == -1);

    // 2. Verify Markov Nilpotent Path Pruning
    TSFiMarkovLM lm;
    tsfi_markov_lm_init(&lm);

    TSFiPruner pruner = {
        .lm = &lm,
        .cumulative_threshold = 0.50f
    };
    tsfi_intrusive_heap_init(&pruner.path_heap);

    // Valid high probability path: T -> O -> N -> E (Prob: 0.90 * 0.95 * 0.98 = 0.8379 > 0.50)
    int path_valid[4] = { PHONEME_T, PHONEME_O, PHONEME_N, PHONEME_E };
    int status = tsfi_shm_prune_path(&pruner, path_valid, 4);
    printf("  [Pruner Valid] Status: %d (Expected 0 - Allowed)\n", status);
    fflush(stdout);
    assert(status == 0);

    // Invalid low probability path: T -> N -> E (Prob: 0.10 * 0.98 = 0.098 < 0.50)
    int path_invalid[3] = { PHONEME_T, PHONEME_N, PHONEME_E };
    status = tsfi_shm_prune_path(&pruner, path_invalid, 3);
    printf("  [Pruner Invalid] Status: %d (Expected 1 - Pruned)\n", status);
    fflush(stdout);
    assert(status == 1);

    // 3. Verify Intrusive Heap integration inside pruner context
    TSFiIntrusiveNode p1 = { .priority = 20, .keycode = 100, .heap_index = -1 };
    TSFiIntrusiveNode p2 = { .priority = 10, .keycode = 200, .heap_index = -1 };
    
    tsfi_shm_prune_push_node(&pruner, &p1);
    tsfi_shm_prune_push_node(&pruner, &p2);
    
    // Update p1 priority (bubble to root)
    tsfi_shm_prune_update_node(&pruner, &p1, 5);
    
    TSFiIntrusiveNode *popped = tsfi_shm_prune_pop_node(&pruner);
    assert(popped != NULL && popped->keycode == 100);
    printf("  [Pruner Intrusive] Popped highest priority path keycode: %d (Expected: 100)\n", popped->keycode);
    fflush(stdout);

    printf("[PASS] Zero-copy register and graph pruning verified successfully!\n");
    fflush(stdout);
    return 0;
}
