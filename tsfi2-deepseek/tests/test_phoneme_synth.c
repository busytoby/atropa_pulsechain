#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_phoneme_synth.h"

int main(void) {
    printf("[Phoneme Synth] Starting direct character-to-synthesizer mapping tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiSynthPerfEngine *perf_engine = tsfi_synth_perf_create(dat, trie_root);
    assert(perf_engine != NULL);

    // Initial frequency of tone-wheel 0 is 440.0Hz
    float base_freq = perf_engine->synth->wheels[0].frequency;

    // 1. Resolve word "tone" (ASCII: 't'=116, 'o'=111, 'n'=110, 'e'=101)
    // Avg ASCII = (116+111+110+101)/4 = 109.5
    // Freq offset = 109.5 * 2.0Hz = 219.0Hz
    // Expected target freq = 440.0Hz + 219.0Hz = 659.0Hz
    int res = tsfi_phoneme_map_word(perf_engine, "tone", 0);
    printf("  [Phoneme Map] Status: %d, Modulated Freq: %.2f Hz (Expected %.2f Hz)\n", 
           res, perf_engine->synth->wheels[0].frequency, base_freq + 219.0f);
    fflush(stdout);
    assert(res == 0);
    assert(fabs(perf_engine->synth->wheels[0].frequency - 659.0f) < 0.0001f);

    // Verify coordinate is logged in AKB cache
    const char *cached_val = tsfi_akb_read(perf_engine->synth->orchestrator->akb, "svdag/phoneme/0");
    printf("  [Phoneme Map] Cached Value: %s (Expected FREQ_659.00)\n", cached_val);
    fflush(stdout);
    assert(cached_val && strcmp(cached_val, "FREQ_659.00") == 0);

    // Cleanup
    tsfi_synth_perf_destroy(perf_engine);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Direct character-to-synthesizer mapping verified successfully!\n");
    fflush(stdout);
    return 0;
}
