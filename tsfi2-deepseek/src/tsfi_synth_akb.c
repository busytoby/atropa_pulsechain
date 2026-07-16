#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_synth_akb.h"

TSFiSynthAKB* tsfi_synth_akb_create(tsfi_dat *disk_dat, tsfi_trie_node *trie_root) {
    if (!trie_root) return NULL;
    TSFiSynthAKB *synth = calloc(1, sizeof(TSFiSynthAKB));
    if (!synth) return NULL;

    synth->orchestrator = tsfi_box_orchestrator_create(disk_dat, trie_root);
    
    // Initialize 4 tone-wheels
    synth->wheels[0] = (TSFiToneWheel){ .frequency = 440.0f, .amplitude = 0.0f }; // A4
    synth->wheels[1] = (TSFiToneWheel){ .frequency = 554.37f, .amplitude = 0.0f }; // C#5
    synth->wheels[2] = (TSFiToneWheel){ .frequency = 659.25f, .amplitude = 0.0f }; // E5
    synth->wheels[3] = (TSFiToneWheel){ .frequency = 880.0f, .amplitude = 0.0f }; // A5
    synth->active_wheel_count = 4;

    return synth;
}

void tsfi_synth_akb_process_input(TSFiSynthAKB *synth, int wheel_idx, float amplitude, const char *prior_path) {
    if (!synth || wheel_idx < 0 || wheel_idx >= synth->active_wheel_count || !prior_path) return;

    // 1. Modulate tone-wheel amplitude (black-box input)
    synth->wheels[wheel_idx].amplitude = amplitude;

    // 2. Map amplitude to AKB prior percentage weight (0.0f - 1.0f -> 0 - 100)
    int weight = (int)(amplitude * 100.0f);
    char weight_str[32];
    snprintf(weight_str, sizeof(weight_str), "%d", weight);

    // 3. Push potential value to AKB cache
    tsfi_akb_write(synth->orchestrator->akb, prior_path, weight_str);
}

int tsfi_synth_akb_tick(TSFiSynthAKB *synth, int x_start, int x_end, int y, int z,
                        const char *hyp_path, const char *ev_path) {
    if (!synth || !synth->orchestrator) return -1;

    // Perform verification step using modulated priors
    return tsfi_box_orchestrator_step(synth->orchestrator, x_start, x_end, y, z, hyp_path, ev_path);
}

void tsfi_synth_akb_destroy(TSFiSynthAKB *synth) {
    if (!synth) return;
    if (synth->orchestrator) tsfi_box_orchestrator_destroy(synth->orchestrator);
    free(synth);
}
