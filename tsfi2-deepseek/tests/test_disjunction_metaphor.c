#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "../../scripts/libantigravity_extra2.c"

// Hash coordinate mapping helper
static int hash_coord(const char *str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % 100;
}

int main(void) {
    printf("[Metaphor Unification] Starting disjunction metaphor loop tests...\n");
    fflush(stdout);

    // 1. Voxelize SVDAG Disjunction Sibling Paths (Metaphor of choice)
    const char *branch_a = "owns_token";
    const char *branch_b = "active_presence";
    
    int xa = hash_coord(branch_a), ya = hash_coord("peer_007"), za = hash_coord("1");
    int xb = hash_coord(branch_b), yb = hash_coord("peer_007"), zb = hash_coord("1");
    
    char path_a[128], path_b[128];
    snprintf(path_a, sizeof(path_a), "svdag/%d/%d/%d", xa, ya, za);
    snprintf(path_b, sizeof(path_b), "svdag/%d/%d/%d", xb, yb, zb);
    printf("  [Metaphor] Sibling A: %s\n", path_a);
    printf("  [Metaphor] Sibling B: %s\n", path_b);
    fflush(stdout);

    // 2. Stochastic Choice Rollout (Simulation in VM)
    // Threshold is 20% (Event A = 50%, Event B = 40% -> Joint = 20%)
    int threshold = 20;
    InteropStackVM vm;
    memset(&vm, 0, sizeof(InteropStackVM));
    
    // Script logic:
    // PUSH -9999    (unbound explanation)
    // PUSH 20       (threshold)
    // PROB_TRY 13    (jump target PC 13 - satisfied by LCG roll of 26)
    // PUSH 101      (bind branch A)
    // PUSH 0
    // OP_BIND
    // OP_FAIL       (forces backtrack to 13, unbinding index 0)
    // HALT
    // Target (PC 13):
    // PUSH 200      (branch B satisfied)
    // HALT
    int script[16] = {
        1, -9999,
        1, threshold,
        0x26, 13,
        1, 101,
        1, 0,
        0x25,
        0x22,
        6,
        1, 200,
        6
    };
    
    assert(interop_stack_vm_execute(&vm, script, 15) == 0);
    printf("  [Simulation] Rollout complete. Stack size: %d, [0]: %d, [1]: %d\n",
           (int)vm.stack_len, vm.stack[0], vm.stack_len > 1 ? vm.stack[1] : 0);
    fflush(stdout);
    assert(vm.stack[0] == -9999);
    assert(vm.stack_len == 2 && vm.stack[1] == 200); // Backtrack resolved branch B

    // 3. Solidify choice to memory-mapped DAT rails disk
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    
    char xb_str[32], yb_str[32], zb_str[32];
    snprintf(xb_str, sizeof(xb_str), "%d", xb);
    snprintf(yb_str, sizeof(yb_str), "%d", yb);
    snprintf(zb_str, sizeof(zb_str), "%d", zb);
    
    struct timespec s, e;
    clock_gettime(CLOCK_MONOTONIC, &s);
    tsfi_dat *dat = tsfi_dat_compile_relation(trie_root, xb_str, yb_str, zb_str);
    assert(dat != NULL);
    assert(tsfi_dat_save_mmap(dat, "tmp/test_metaphor_rails.dat.bin") == 0);
    clock_gettime(CLOCK_MONOTONIC, &e);
    
    long long duration_ns = (e.tv_sec - s.tv_sec) * 1000000000LL + (e.tv_nsec - s.tv_nsec);
    printf("  [Solidification] Saved to rails file in %lld ns.\n", duration_ns);
    fflush(stdout);

    // Verify lookup
    char query[128];
    snprintf(query, sizeof(query), "%d/%d/%d", xb, yb, zb);
    const char *res = tsfi_dat_search(dat, query);
    assert(res != NULL && strcmp(res, "RELATION_TRUE") == 0);
    
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Metaphors of disjunction successfully unified and verified!\n");
    fflush(stdout);
    return 0;
}
