#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "../../scripts/libantigravity_extra2.c"

int main(void) {
    printf("[Horn Abduction] Starting Monte Carlo abduction test...\n");
    fflush(stdout);

    // 1. Setup Abducible Weights inside Trie
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_trie_insert(trie_root, "abduce/peer_007/has_token", "50");       // P(H1) = 50%
    tsfi_trie_insert(trie_root, "abduce/peer_007/active_presence", "40"); // P(H2) = 40%
    
    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    // 2. Query Abducible Confidence Metrics
    const char *h1_str = tsfi_dat_search(dat, "abduce/peer_007/has_token");
    const char *h2_str = tsfi_dat_search(dat, "abduce/peer_007/active_presence");
    assert(h1_str != NULL && h2_str != NULL);
    
    int p_h1 = atoi(h1_str);
    int p_h2 = atoi(h2_str);
    int joint_prob = (p_h1 * p_h2) / 100; // Joint probability of explanations = 40%
    
    printf("[Horn Abduction] Abducing explanation for transaction_validated(peer_007)\n");
    printf("[Horn Abduction] H1 Probability: %d%%, H2 Probability: %d%%, Joint Probability: %d%%\n", 
           p_h1, p_h2, joint_prob);
    fflush(stdout);
    assert(joint_prob == 20);

    // 3. Monte Carlo Abduction Script Execution
    // Script logic:
    // PUSH -9999    (unbound explanation placeholder)
    // PUSH 40       (joint hypothesis probability threshold)
    // PROB_TRY 12    (target PC 12 - satisfied by LCG roll of 26)
    // PUSH 101      (abduce/assume explanation true)
    // PUSH 0        (index 0)
    // OP_BIND       (binds explanation to 101, registers on Trail)
    // OP_FAIL       (forces backtrack to 12, unbinding index 0)
    // HALT
    // Target (Index 12):
    // PUSH 200      (alternative explanation resolved)
    // HALT
    InteropStackVM vm;
    memset(&vm, 0, sizeof(InteropStackVM));
    
    int abduction_script[16] = {
        1, -9999,
        1, joint_prob,
        0x26, 13,
        1, 101,
        1, 0,
        0x25,
        0x22,
        6,
        1, 200,
        6
    };
    
    assert(interop_stack_vm_execute(&vm, abduction_script, 15) == 0);
    printf("[Horn Abduction] VM execution completed. Stack size: %d, [0]: %d, [1]: %d\n", 
           (int)vm.stack_len, vm.stack[0], vm.stack_len > 1 ? vm.stack[1] : 0);
    fflush(stdout);

    // Assert that the abduced explanation was unbound correctly, and target PC reached
    assert(vm.stack[0] == -9999);
    assert(vm.stack_len == 2 && vm.stack[1] == 200);

    // Cleanup
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Horn Abduction Monte Carlo solver verified successfully!\n");
    fflush(stdout);
    return 0;
}
