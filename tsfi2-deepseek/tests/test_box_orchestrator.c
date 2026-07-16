#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_box_orchestrator.h"

int main(void) {
    printf("[Box Orchestrator] Starting unified box orchestrator integration tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    // Setup initial SVDAG frontier: x=0 is RELATION_TRUE, x=1 is QUESTION_PENDING
    tsfi_trie_insert(trie_root, "svdag/0/10/10", "RELATION_TRUE");
    tsfi_trie_insert(trie_root, "svdag/1/10/10", "QUESTION_PENDING");

    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiBoxOrchestrator *orc = tsfi_box_orchestrator_create(dat, trie_root);
    assert(orc != NULL);

    // Run unified 4-box step execution
    // Expected behavior:
    // 1. Blue Box loads priors: P(H) = 60%, P(E) = 50%, P(H AND E) = 40%
    // 2. Yellow Box detects CLASS_FRONTIER on svdag/0/10/10 -> svdag/1/10/10 transition.
    // 3. Green Box Markov VM evaluates weight checks (0.50 + 0.40 = 0.90 >= posterior_limit 0.80) -> returns 1.
    // 4. Red Box crystallizes svdag/1/10/10 to RELATION_TRUE.
    int status = tsfi_box_orchestrator_step(orc, 0, 1, 10, 10, "prior/holds_token", "prior/active");
    printf("  [Orchestrator] Step status: %d (Expected 1=Resolved), Resolved count: %d\n", status, orc->resolved_count);
    fflush(stdout);
    assert(status == 1);
    assert(orc->resolved_count == 1);

    // Verify coordinates are now stabilized to RELATION_TRUE on the recompiled DAT
    const char *v1 = tsfi_dat_search(orc->akb->disk_dat, "svdag/1/10/10");
    printf("  [Post State] Voxel svdag/1/10/10 value: %s (Expected RELATION_TRUE)\n", v1);
    fflush(stdout);
    assert(v1 && strcmp(v1, "RELATION_TRUE") == 0);

    // Cleanup
    tsfi_box_orchestrator_destroy(orc);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Unified box orchestrator loop verified successfully!\n");
    fflush(stdout);
    return 0;
}
