#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_akb.h"

int main(void) {
    printf("[AKB] Starting Advanced Knowledge Base integration tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_trie_insert(trie_root, "akb/prior", "50");

    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    TSFiAKB *akb = tsfi_akb_create(dat);
    assert(akb != NULL);

    // 1. Verify Cache-First Fast Write/Read (Dirty-Page Buffers)
    tsfi_akb_write(akb, "akb/dirty_key", "99");
    const char *cached_val = tsfi_akb_read(akb, "akb/dirty_key");
    printf("  [Cache] Read written key: %s (Expected 99)\n", cached_val);
    fflush(stdout);
    assert(cached_val && strcmp(cached_val, "99") == 0);

    // 1b. Verify Ephemeral Temporary State construction
    tsfi_akb_write_temp(akb, "akb/temp_key", "RELATION_TRUE");
    assert(tsfi_akb_is_temp(akb, "akb/temp_key") == 1);
    const char *temp_val = tsfi_akb_read(akb, "akb/temp_key");
    printf("  [Cache] Read temporary key: %s (Expected TEMP_RELATION_TRUE)\n", temp_val);
    fflush(stdout);
    assert(temp_val && strcmp(temp_val, "TEMP_RELATION_TRUE") == 0);

    // Verify sync page flushing (crystallizing temp values to permanent rails)
    tsfi_akb_sync(akb, trie_root);
    // Reload compiled search directly to verify persistence and crystallization
    const char *persisted_val = tsfi_dat_search(akb->disk_dat, "akb/dirty_key");
    const char *persisted_temp = tsfi_dat_search(akb->disk_dat, "akb/temp_key");
    assert(persisted_val && strcmp(persisted_val, "99") == 0);
    assert(persisted_temp && strcmp(persisted_temp, "RELATION_TRUE") == 0);
    printf("  [Cache] Verified sync successfully crystallized temporary weights to permanent rails.\n");
    fflush(stdout);

    // 2. Verify Secondary Graph Adjacency index
    tsfi_akb_add_edge(akb, "Node_A", "Node_B", 0.75f);
    float edge_w = tsfi_akb_get_edge(akb, "Node_A", "Node_B");
    printf("  [Graph] Edge weight Node_A -> Node_B: %.2f (Expected 0.75)\n", edge_w);
    fflush(stdout);
    assert(fabs(edge_w - 0.75f) < 0.0001f);

    // 3. Verify Markov Logic VM execution (OP_WEIGHT / OP_SAT)
    TSFiMarkovVM vm;
    memset(&vm, 0, sizeof(TSFiMarkovVM));

    // bytecode:
    // PUSH 50 (weight 0.50)
    // OP_WEIGHT
    // PUSH 30 (weight 0.30)
    // OP_WEIGHT
    // PUSH 70 (limit weight 0.70)
    // OP_SAT (total 0.50 + 0.30 = 0.80 >= 0.70 -> Pushes 1)
    // HALT
    int bytecode[11] = {
        1, 50,
        0x31,
        1, 30,
        0x31,
        1, 70,
        0x32,
        6
    };

    assert(tsfi_markov_vm_execute(&vm, bytecode, 11) == 0);
    printf("  [Markov VM] Sat status: %d (Expected 1)\n", vm.stack[0]);
    fflush(stdout);
    assert(vm.stack[0] == 1);

    // Cleanup
    tsfi_akb_destroy(akb);
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Advanced Knowledge Base model verified successfully!\n");
    fflush(stdout);
    return 0;
}
