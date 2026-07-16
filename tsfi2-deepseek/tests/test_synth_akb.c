#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_synth_akb.h"

int main(void) {
    printf("[Synth AKB] Starting synthesizer tone-wheel AKB integration tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    // Setup initial SVDAG frontier: x=0 is RELATION_TRUE, x=1 is QUESTION_PENDING
    tsfi_trie_insert(trie_root, "svdag/0/10/10", "RELATION_TRUE");
    tsfi_trie_insert(trie_root, "svdag/1/10/10", "QUESTION_PENDING");

    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiSynthAKB *synth = tsfi_synth_akb_create(dat, trie_root);
    assert(synth != NULL);

    // 1. Simulate Black-Box input modulating tone-wheel amplitudes
    // Map amplitudes to hypothesis and evidence prior weights
    tsfi_synth_akb_process_input(synth, 0, 0.60f, "prior/holds_token"); // Maps 0.60 to 60%
    tsfi_synth_akb_process_input(synth, 1, 0.50f, "prior/active");      // Maps 0.50 to 50%

    // 2. Execute Synthesizer Tick to trigger logical deduction
    int status = tsfi_synth_akb_tick(synth, 0, 1, 10, 10, "prior/holds_token", "prior/active");
    printf("  [Synth AKB] Tick status: %d (Expected 1=Resolved)\n", status);
    fflush(stdout);
    assert(status == 1);

    // Verify coordinate crystallized to permanent DAT rails post deduction
    const char *val = tsfi_dat_search(synth->orchestrator->akb->disk_dat, "svdag/1/10/10");
    printf("  [Post State] Voxel svdag/1/10/10: %s (Expected RELATION_TRUE)\n", val);
    fflush(stdout);
    assert(val && strcmp(val, "RELATION_TRUE") == 0);

    // Cleanup
    tsfi_synth_akb_destroy(synth);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Synthesizer tone-wheel AKB integration verified successfully!\n");
    fflush(stdout);
    return 0;
}
