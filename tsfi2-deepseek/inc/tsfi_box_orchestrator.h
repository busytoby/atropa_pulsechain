#ifndef TSFI_BOX_ORCHESTRATOR_H
#define TSFI_BOX_ORCHESTRATOR_H

#include "tsfi_akb.h"
#include "tsfi_unified_classifier.h"

// Unified Box System State
typedef struct {
    TSFiAKB *akb;
    tsfi_trie_node *trie_root;
    int resolved_count;
    int failed_count;
} TSFiBoxOrchestrator;

// Initialize the Box Orchestrator with an active DAT database
TSFiBoxOrchestrator* tsfi_box_orchestrator_create(tsfi_dat *disk_dat, tsfi_trie_node *trie_root);

// Run the unified 4-box loop over a coordinate region to resolve logic strains
int tsfi_box_orchestrator_step(TSFiBoxOrchestrator *orc, int x_start, int x_end, int y, int z,
                               const char *hyp_path, const char *ev_path);

// Destroy the Box Orchestrator structures
void tsfi_box_orchestrator_destroy(TSFiBoxOrchestrator *orc);

#endif // TSFI_BOX_ORCHESTRATOR_H
