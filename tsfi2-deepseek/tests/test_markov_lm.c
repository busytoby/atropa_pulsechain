#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_markov_lm.h"

int main(void) {
    printf("[Markov LM] Starting Markov language model sequence tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiSynthPerfEngine *perf_engine = tsfi_synth_perf_create(dat, trie_root);
    assert(perf_engine != NULL);

    TSFiMarkovLM lm;
    tsfi_markov_lm_init(&lm);

    TSFiDualStackVM vm;
    memset(&vm, 0, sizeof(TSFiDualStackVM));

    int current = PHONEME_T;

    // 1. First step: T -> O (Prob: 0.90, exceeds threshold 0.50)
    int res = tsfi_markov_lm_step(&lm, perf_engine, &vm, &current, 0.50f);
    printf("  [Markov Transition 1] Status: %d, Next Phoneme: %d (Expected PHONEME_O=1)\n", res, current);
    fflush(stdout);
    assert(res == 0);
    assert(current == PHONEME_O);
    assert(fabs(perf_engine->synth->wheels[0].frequency - 480.0f) < 0.0001f);

    // 2. Second step: O -> N (Prob: 0.95, exceeds threshold 0.50)
    res = tsfi_markov_lm_step(&lm, perf_engine, &vm, &current, 0.50f);
    printf("  [Markov Transition 2] Status: %d, Next Phoneme: %d (Expected PHONEME_N=2)\n", res, current);
    fflush(stdout);
    assert(res == 0);
    assert(current == PHONEME_N);
    assert(fabs(perf_engine->synth->wheels[0].frequency - 520.0f) < 0.0001f);

    // 3. Third step: High threshold (0.99) triggers constraint violation (Prob O->N = 0.95 < 0.99)
    // Revert current to O to test fail case
    current = PHONEME_O;
    res = tsfi_markov_lm_step(&lm, perf_engine, &vm, &current, 0.99f);
    printf("  [Markov Transition Fail] Status: %d (Expected -2), Backtrack Flag: %d (Expected 1)\n", res, vm.backtrack_triggered);
    fflush(stdout);
    assert(res == -2);
    assert(vm.backtrack_triggered == 1);

    // Cleanup
    tsfi_synth_perf_destroy(perf_engine);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Markov language model sequence verified successfully!\n");
    fflush(stdout);
    return 0;
}
