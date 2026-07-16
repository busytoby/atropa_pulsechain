#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"

int main(void) {
    printf("[Datalog Unification] Starting relational compiler benchmark...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    
    // Measure compiler latency for relation path compiling
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    tsfi_dat *rel_dat = tsfi_dat_compile_relation(trie_root, "owns_wallet", "agent_007", "0xf39fd6e51aad88f6f4ce6ab8827279cfffb92266");
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    assert(rel_dat != NULL);
    
    long long duration_ns = (end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
    printf("[Datalog Unification] Relation compiled in %lld ns.\n", duration_ns);
    fflush(stdout);
    
    // Search check
    const char *res = tsfi_dat_search(rel_dat, "owns_wallet/agent_007/0xf39fd6e51aad88f6f4ce6ab8827279cfffb92266");
    assert(res != NULL && strcmp(res, "RELATION_TRUE") == 0);
    
    tsfi_dat_destroy(rel_dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Datalog relation unification compiled and resolved successfully!\n");
    fflush(stdout);
    return 0;
}
