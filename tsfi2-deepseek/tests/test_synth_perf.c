#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_synth_perf.h"

int main(void) {
    printf("[Synth Perf] Starting synthesizer performance engine integration tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiSynthPerfEngine *engine = tsfi_synth_perf_create(dat, trie_root);
    assert(engine != NULL);

    // 1. Test Lock-Free Audio Ring Buffer
    tsfi_synth_perf_push_audio(engine, 0.5f);
    tsfi_synth_perf_push_audio(engine, -0.2f);
    float a1 = tsfi_synth_perf_pop_audio(engine);
    float a2 = tsfi_synth_perf_pop_audio(engine);
    printf("  [Audio] Popped samples: %.1f, %.1f\n", a1, a2);
    fflush(stdout);
    assert(fabs(a1 - 0.5f) < 0.0001f);
    assert(fabs(a2 - (-0.2f)) < 0.0001f);

    // 2. Test Vulkan VRAM Direct Mapping
    float simulated_vram[10] = {0.0f};
    tsfi_synth_perf_map_vram(engine, simulated_vram, 10);
    // Write belief weights to cache
    tsfi_akb_write(engine->synth->orchestrator->akb, "prior/holds_token", "85");
    tsfi_synth_perf_sync_vram(engine);
    printf("  [VRAM] Direct mapped sync output: %.2f (Expected 85.00)\n", simulated_vram[0]);
    fflush(stdout);
    assert(fabs(simulated_vram[0] - 85.0f) < 0.0001f);

    // 3. Test Parallel Constraint Solver (Multi-threaded OpenMP)
    int bytecode[11] = {
        1, 50,
        0x31,
        1, 30,
        0x31,
        1, 70,
        0x32,
        6
    };
    int results[4] = {0};
    tsfi_synth_perf_parallel_solve(engine, bytecode, 11, 4, results);
    printf("  [Solver] Parallel branches resolved sat: %d %d %d %d\n", results[0], results[1], results[2], results[3]);
    fflush(stdout);
    for (int i = 0; i < 4; i++) {
        assert(results[i] == 1);
    }

    // Cleanup
    tsfi_synth_perf_destroy(engine);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Synthesizer performance engine verified successfully!\n");
    fflush(stdout);
    return 0;
}
