#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_algebraic_query.h"
#include "../../scripts/libantigravity_extra2.c"

int main(void) {
    printf("[Markov Selection] Starting Markov path selection tests...\n");
    fflush(stdout);

    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    
    // Register transition probabilities (weights 0-100)
    tsfi_trie_insert(trie_root, "markov/p12", "50"); // P(1 -> 2) = 50%
    tsfi_trie_insert(trie_root, "markov/p23", "60"); // P(2 -> 3) = 60%

    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    // Build algebraic path query node: P(1 -> 2) AND P(2 -> 3)
    TSFiQueryNode *p12 = tsfi_query_create_var("markov/p12");
    TSFiQueryNode *p23 = tsfi_query_create_var("markov/p23");
    TSFiQueryNode *path_query = tsfi_query_create_mul(p12, p23);

    float path_prob = tsfi_query_evaluate(path_query, dat);
    int path_threshold = (int)(path_prob * 100.0f);
    printf("  [Markov] Path probability: %.2f (%d%%)\n", path_prob, path_threshold);
    fflush(stdout);
    assert(path_threshold == 30); // 0.50 * 0.60 = 0.30

    // VM Simulation:
    // PUSH -777     (Placeholder)
    // PUSH 30       (Path threshold)
    // PROB_TRY 13   (Target PC 13 - fails because LCG roll 26 < 30)
    // PUSH 888      (Branch A - taken on failure)
    // HALT
    // Target (PC 13):
    // PUSH 999      (Branch B - taken on success)
    // HALT
    InteropStackVM vm;
    memset(&vm, 0, sizeof(InteropStackVM));

    int script[12] = {
        1, -777,
        1, path_threshold,
        0x26, 9,
        1, 888,
        6,
        1, 999,
        6
    };

    assert(interop_stack_vm_execute(&vm, script, 11) == 0);
    printf("  [VM Execution] Stack output: %d, [1]: %d\n", vm.stack[0], vm.stack[1]);
    fflush(stdout);
    
    // With LCG roll of 26, the 30% threshold check fails.
    // So the jump to PC 9 is bypassed, executing Branch A (888).
    assert(vm.stack[0] == -777);
    assert(vm.stack[1] == 888);

    // Cleanup
    tsfi_query_destroy(path_query);
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Markov path selection model verified successfully!\n");
    fflush(stdout);
    return 0;
}
