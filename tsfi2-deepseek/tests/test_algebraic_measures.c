#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "../../scripts/libantigravity_extra2.c"

int main(void) {
    printf("[Algebraic Measures] Starting algebraic probability measure tests...\n");
    fflush(stdout);

    // 1. Setup trie parameters representing independent event weights
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_trie_insert(trie_root, "events/owns_token", "50");       // P(A) = 50%
    tsfi_trie_insert(trie_root, "events/active_presence", "40"); // P(B) = 40%
    tsfi_trie_insert(trie_root, "events/signature_valid", "80"); // P(C) = 80%
    
    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);

    // 2. Fetch and evaluate joint algebraic intersection
    const char *pa_str = tsfi_dat_search(dat, "events/owns_token");
    const char *pb_str = tsfi_dat_search(dat, "events/active_presence");
    const char *pc_str = tsfi_dat_search(dat, "events/signature_valid");
    assert(pa_str && pb_str && pc_str);

    double pa = atof(pa_str) / 100.0;
    double pb = atof(pb_str) / 100.0;
    double pc = atof(pc_str) / 100.0;

    // Joint probability measure calculation: P(A * B * C)
    double joint_measure = pa * pb * pc;
    int joint_threshold = (int)(joint_measure * 100.0);
    printf("  [Algebraic] pa=%.2f, pb=%.2f, pc=%.2f -> Joint Measure = %.2f (%d%%)\n",
           pa, pb, pc, joint_measure, joint_threshold);
    fflush(stdout);
    assert(joint_threshold == 16);

    // 3. Execute VM Stochastic Sampling
    InteropStackVM vm;
    memset(&vm, 0, sizeof(InteropStackVM));
    
    // Script logic:
    // PUSH -9999    (placeholder)
    // PUSH 16       (joint_threshold)
    // PROB_TRY 13   (Target PC 13 - satisfied by LCG roll of 26 >= 16)
    // PUSH 101      (Branch A)
    // PUSH 0
    // OP_BIND
    // OP_FAIL       (forces backtrack to 13, unbinding index 0)
    // HALT
    // Target (PC 13):
    // PUSH 200      (Branch B satisfied)
    // HALT
    int script[16] = {
        1, -9999,
        1, joint_threshold,
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

    // Cleanup
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Algebraic probability measures verified successfully!\n");
    fflush(stdout);
    return 0;
}
