#include <stdio.h>
#include <string.h>
#include "tsfi_synth_direct.h"

const char* tsfi_synth_direct_read(const TSFiSynthAKB *synth, const char *key) {
    if (!synth || !synth->orchestrator || !synth->orchestrator->akb || !key) return NULL;

    TSFiAKB *akb = synth->orchestrator->akb;
    
    // Bypass trie and read directly from cache dirty page rails
    for (int i = 0; i < akb->cache_count; i++) {
        if (strcmp(akb->cache[i].key, key) == 0) {
            return akb->cache[i].value;
        }
    }

    return NULL;
}

float tsfi_synth_direct_get_edge(const TSFiSynthAKB *synth, const char *src, const char *dest) {
    if (!synth || !synth->orchestrator || !synth->orchestrator->akb) return 0.0f;
    return tsfi_akb_get_edge(synth->orchestrator->akb, src, dest);
}
