#ifndef TSFI_SYNTH_AKB_H
#define TSFI_SYNTH_AKB_H

#include "tsfi_box_orchestrator.h"

// Synthesizer Tone-Wheel structure
typedef struct {
    float frequency;
    float amplitude; // Range [0.0f, 1.0f]
} TSFiToneWheel;

// Synthesizer AKB Integration State
typedef struct {
    TSFiBoxOrchestrator *orchestrator;
    TSFiToneWheel wheels[4];
    int active_wheel_count;
} TSFiSynthAKB;

// Initialize Synthesizer AKB Integrator
TSFiSynthAKB* tsfi_synth_akb_create(tsfi_dat *disk_dat, tsfi_trie_node *trie_root);

// Process dynamic black-box input (modulates tone-wheel amplitude and maps to AKB priors)
void tsfi_synth_akb_process_input(TSFiSynthAKB *synth, int wheel_idx, float amplitude, const char *prior_path);

// Run logical deduction cycle driven by synthesizer signals
int tsfi_synth_akb_tick(TSFiSynthAKB *synth, int x_start, int x_end, int y, int z,
                        const char *hyp_path, const char *ev_path);

// Destroy Synthesizer AKB structures
void tsfi_synth_akb_destroy(TSFiSynthAKB *synth);

#endif // TSFI_SYNTH_AKB_H
