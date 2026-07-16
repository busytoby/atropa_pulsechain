#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_adsr_trie.h"

void tsfi_adsr_init(TSFiADSR *adsr, float a, float d, float s, float r) {
    if (!adsr) return;
    adsr->attack_time = a;
    adsr->decay_time = d;
    adsr->sustain_level = s;
    adsr->release_time = r;
}

float tsfi_adsr_calculate_amplitude(const TSFiADSR *adsr, float time, float key_release_time) {
    if (!adsr || time < 0.0f) return 0.0f;

    // Release phase
    if (key_release_time >= 0.0f && time >= key_release_time) {
        float rel_elapsed = time - key_release_time;
        if (rel_elapsed >= adsr->release_time) return 0.0f;
        float start_amp = tsfi_adsr_calculate_amplitude(adsr, key_release_time, -1.0f);
        return start_amp * (1.0f - (rel_elapsed / adsr->release_time));
    }

    // Attack phase
    if (time < adsr->attack_time) {
        if (adsr->attack_time <= 0.0f) return 1.0f;
        return time / adsr->attack_time;
    }

    // Decay phase
    float decay_elapsed = time - adsr->attack_time;
    if (decay_elapsed < adsr->decay_time) {
        if (adsr->decay_time <= 0.0f) return adsr->sustain_level;
        float diff = 1.0f - adsr->sustain_level;
        return 1.0f - (diff * (decay_elapsed / adsr->decay_time));
    }

    // Sustain phase
    return adsr->sustain_level;
}

TSFiLexicalTrie* tsfi_lexical_trie_create(void) {
    TSFiLexicalTrie *dict = calloc(1, sizeof(TSFiLexicalTrie));
    if (!dict) return NULL;
    dict->root = tsfi_trie_create_node(0);
    return dict;
}

void tsfi_lexical_trie_insert(TSFiLexicalTrie *dict, const char *word) {
    if (!dict || !dict->root || !word) return;
    tsfi_trie_insert(dict->root, word, "VALID");
}

int tsfi_lexical_trie_contains(TSFiLexicalTrie *dict, const char *word) {
    if (!dict || !dict->root || !word) return 0;
    const char *res = tsfi_trie_lookup(dict->root, word);
    return (res && strcmp(res, "VALID") == 0) ? 1 : 0;
}

void tsfi_lexical_trie_destroy(TSFiLexicalTrie *dict) {
    if (!dict) return;
    if (dict->root) {
        tsfi_trie_destroy(dict->root);
    }
    free(dict);
}
