#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "tsfi_ring_buffer.h"
#include "../../scripts/libantigravity_extra2.c"

int main(void) {
    printf("[PLP Solver] Starting Probabilistic Logic Programming tests...\n");
    fflush(stdout);

    // 1. Compile Weighted Fact Trie
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    
    // We register two independent probability weights (expressed as integers 0..100)
    tsfi_trie_insert(trie_root, "prob/peer_007/verified", "50"); // P(A) = 50%
    tsfi_trie_insert(trie_root, "prob/peer_007/active", "40");   // P(B) = 40%
    
    tsfi_dat *prob_dat = tsfi_dat_compile(trie_root);
    assert(prob_dat != NULL);

    // 2. Query and Unify Weights
    const char *p_a_str = tsfi_dat_search(prob_dat, "prob/peer_007/verified");
    const char *p_b_str = tsfi_dat_search(prob_dat, "prob/peer_007/active");
    assert(p_a_str != NULL && p_b_str != NULL);
    
    int p_a = atoi(p_a_str);
    int p_b = atoi(p_b_str);
    
    // Compute joint probability: P(A and B) = P(A) * P(B) / 100 = 30%
    int joint_prob = (p_a * p_b) / 100;
    printf("[PLP Solver] Event A: %d%%, Event B: %d%%, Joint Probability: %d%%\n", p_a, p_b, joint_prob);
    unsigned int rand_seed = 98765;
    rand_seed = rand_seed * 1103515245 + 12345;
    int roll = (rand_seed / 65536) % 100;
    printf("[PLP Solver] Deterministic LCG roll: %d\n", roll);
    fflush(stdout);
    assert(joint_prob == 20);

    // 3. Execute Stochastic Choice-Point VM Script
    InteropStackVM vm;
    memset(&vm, 0, sizeof(InteropStackVM));
    
    int plp_script[16] = {
        1, -9999,
        1, joint_prob,
        0x26, 13,
        1, 555,
        1, 0,
        0x25,
        0x22,
        6,
        1, 999,
        6
    };
    
    assert(interop_stack_vm_execute(&vm, plp_script, 16) == 0);
    printf("[PLP Solver] VM execution completed. Stack size: %d, [0]: %d, [1]: %d\n", 
           (int)vm.stack_len, vm.stack[0], vm.stack_len > 1 ? vm.stack[1] : 0);
    fflush(stdout);
    
    // Asserts variable at 0 was unbound correctly to -9999, and target branch pushed 999
    assert(vm.stack[0] == -9999);
    assert(vm.stack_len == 2 && vm.stack[1] == 999);

    // Cleanup
    tsfi_dat_destroy(prob_dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Probabilistic logic path solving completed successfully!\n");
    fflush(stdout);
    return 0;
}
