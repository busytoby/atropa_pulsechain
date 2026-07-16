#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_synth_akb.h"
#include "tsfi_synth_direct.h"

int main(void) {
    printf("[Synth Direct] Starting synthesizer direct knowledge base access tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiSynthAKB *synth = tsfi_synth_akb_create(dat, trie_root);
    assert(synth != NULL);

    // 1. Write value to cache and read directly bypassing disk search
    tsfi_synth_akb_process_input(synth, 0, 0.85f, "prior/holds_token"); // Writes 85%

    const char *direct_val = tsfi_synth_direct_read(synth, "prior/holds_token");
    printf("  [Direct Read] Value: %s (Expected 85)\n", direct_val);
    fflush(stdout);
    assert(direct_val && strcmp(direct_val, "85") == 0);

    // 2. Add adjacency edge and query directly
    tsfi_akb_add_edge(synth->orchestrator->akb, "Tone_A", "Filter_B", 0.90f);
    float edge_w = tsfi_synth_direct_get_edge(synth, "Tone_A", "Filter_B");
    printf("  [Direct Graph] Edge: %.2f (Expected 0.90)\n", edge_w);
    fflush(stdout);
    assert(fabs(edge_w - 0.90f) < 0.0001f);

    // Cleanup
    tsfi_synth_akb_destroy(synth);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Synthesizer direct knowledge base access verified successfully!\n");
    fflush(stdout);
    return 0;
}
