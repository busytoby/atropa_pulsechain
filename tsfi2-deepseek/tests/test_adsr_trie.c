#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_adsr_trie.h"

int main(void) {
    printf("[ADSR & Lexical Trie] Starting ADSR calculation and dictionary lookup tests...\n");
    fflush(stdout);

    // 1. Verify ADSR calculations
    TSFiADSR adsr;
    tsfi_adsr_init(&adsr, 0.1f, 0.2f, 0.7f, 0.3f); // Attack: 0.1s, Decay: 0.2s, Sustain: 0.7, Release: 0.3s

    // At t=0.05s (midway of Attack), amp should be 0.5
    float amp = tsfi_adsr_calculate_amplitude(&adsr, 0.05f, -1.0f);
    printf("  [ADSR Attack] Amp at 0.05s: %.2f (Expected 0.50)\n", amp);
    fflush(stdout);
    assert(fabs(amp - 0.50f) < 0.0001f);

    // At t=0.20s (midway of Decay, which is 0.1s to 0.3s), amp should be midway of 1.0 -> 0.7 (0.85)
    amp = tsfi_adsr_calculate_amplitude(&adsr, 0.20f, -1.0f);
    printf("  [ADSR Decay] Amp at 0.20s: %.2f (Expected 0.85)\n", amp);
    fflush(stdout);
    assert(fabs(amp - 0.85f) < 0.0001f);

    // At t=0.40s (Sustain phase), amp should be 0.70
    amp = tsfi_adsr_calculate_amplitude(&adsr, 0.40f, -1.0f);
    printf("  [ADSR Sustain] Amp at 0.40s: %.2f (Expected 0.70)\n", amp);
    fflush(stdout);
    assert(fabs(amp - 0.70f) < 0.0001f);

    // At t=0.55s (Release phase, trigger release at t=0.40s), amp should decay from 0.7 by half over 0.15s (0.35)
    amp = tsfi_adsr_calculate_amplitude(&adsr, 0.55f, 0.40f);
    printf("  [ADSR Release] Amp at 0.55s: %.2f (Expected 0.35)\n", amp);
    fflush(stdout);
    assert(fabs(amp - 0.35f) < 0.0001f);

    // 2. Verify Lexical Trie operations
    TSFiLexicalTrie *dict = tsfi_lexical_trie_create();
    assert(dict != NULL);

    tsfi_lexical_trie_insert(dict, "tone");
    tsfi_lexical_trie_insert(dict, "synthesizer");

    int found = tsfi_lexical_trie_contains(dict, "tone");
    printf("  [Lexical Contains] 'tone' found: %d (Expected 1)\n", found);
    fflush(stdout);
    assert(found == 1);

    found = tsfi_lexical_trie_contains(dict, "invalid_word");
    printf("  [Lexical Contains] 'invalid_word' found: %d (Expected 0)\n", found);
    fflush(stdout);
    assert(found == 0);

    // Cleanup
    tsfi_lexical_trie_destroy(dict);

    printf("[PASS] ADSR calculation and dictionary lookup verified successfully!\n");
    fflush(stdout);
    return 0;
}
