#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_doxastic_classifier.h"

int main(void) {
    printf("[Peer Abduction] Starting peer challenge and resolution tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);

    // 1. Initial State: Setup pending question representing a frontier
    tsfi_trie_insert(trie_root, "svdag/0/99/99", "RELATION_TRUE");
    tsfi_trie_insert(trie_root, "svdag/1/99/99", "QUESTION_PENDING");

    tsfi_dat *dat1 = tsfi_dat_compile(trie_root);
    assert(dat1 != NULL);

    TSFiStrainClassification s1 = tsfi_doxastic_classify_strain(dat1, 0, 1, 99, 99);
    printf("  [Initial State] Boundary Class: %d (Expected CLASS_FRONTIER=%d)\n", s1.class_id, CLASS_FRONTIER);
    fflush(stdout);
    assert(s1.class_id == CLASS_FRONTIER);

    // 2. Publish Challenge (Simulated via trie registration of the unsolved hash)
    // Key format: challenge/peer_abduction/<hash>
    tsfi_trie_insert(trie_root, "challenge/peer_abduction/0xabc123", "ACTIVE");
    tsfi_dat *dat_challenge = tsfi_dat_compile(trie_root);
    assert(dat_challenge != NULL);

    const char *chal_status1 = tsfi_dat_search(dat_challenge, "challenge/peer_abduction/0xabc123");
    printf("  [Publish] Challenge 0xabc123 status: %s\n", chal_status1);
    fflush(stdout);
    assert(chal_status1 != NULL && strcmp(chal_status1, "ACTIVE") == 0);

    // 3. Simulated Peer Solver resolves the challenge
    printf("  [Peer Solver] Verifying signature and writing crystallized fact...\n");
    fflush(stdout);
    // Peer writes the crystallized fact
    tsfi_trie_insert(trie_root, "svdag/1/99/99", "RELATION_TRUE");
    // Peer marks the challenge resolved (retracted/deleted)
    tsfi_trie_insert(trie_root, "challenge/peer_abduction/0xabc123", "");

    // 4. Local Consolidation: Re-compile and verify stabilization
    tsfi_dat *dat2 = tsfi_dat_compile(trie_root);
    assert(dat2 != NULL);

    const char *chal_status2 = tsfi_dat_search(dat2, "challenge/peer_abduction/0xabc123");
    assert(chal_status2 == NULL || strcmp(chal_status2, "") == 0);
    printf("  [Consolidation] Verified challenge resolved (removed from trie)\n");
    fflush(stdout);

    TSFiStrainClassification s2 = tsfi_doxastic_classify_strain(dat2, 0, 1, 99, 99);
    printf("  [Post State] Boundary Class: %d (Expected CLASS_STABLE=%d)\n", s2.class_id, CLASS_STABLE);
    fflush(stdout);
    assert(s2.class_id == CLASS_STABLE);

    // Cleanup
    tsfi_dat_destroy(dat1);
    tsfi_dat_destroy(dat_challenge);
    tsfi_dat_destroy(dat2);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Peer challenge and distributed logic resolution verified successfully!\n");
    fflush(stdout);
    return 0;
}
