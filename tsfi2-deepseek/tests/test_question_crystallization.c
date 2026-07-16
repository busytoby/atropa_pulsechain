#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "../../scripts/libantigravity_extra2.c"

int main(void) {
    printf("[Crystallization] Starting question-asking crystallization tests...\n");
    fflush(stdout);

    // 1. Setup Question Sibling path in trie (High potential strain)
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_trie_insert(trie_root, "question/owns_token/peer_007/1", "QUESTION_PENDING");
    
    tsfi_dat *dat1 = tsfi_dat_compile(trie_root);
    assert(dat1 != NULL);
    
    // Verify it exists as pending
    const char *q1 = tsfi_dat_search(dat1, "question/owns_token/peer_007/1");
    assert(q1 != NULL && strcmp(q1, "QUESTION_PENDING") == 0);
    printf("  [Step 1] Registered pending question: owns_token(peer_007) is QUESTION_PENDING\n");
    fflush(stdout);

    // 2. VM query blocks on pending question and backtracks
    InteropStackVM vm;
    memset(&vm, 0, sizeof(InteropStackVM));
    
    // Script logic:
    // PUSH -9999    (placeholder)
    // TRY_ME_ELSE 12 (target branch B)
    // PUSH 1        (simulate query match fails because path is QUESTION_PENDING)
    // PUSH 0        (expected success token)
    // OP_NEQ        (fails, triggers backtrack to 12)
    // HALT
    // Target (PC 12):
    // PUSH 999      (alternate search path executed)
    // HALT
    int script[14] = {
        1, -9999,
        0x21, 11,
        1, 1,
        1, 1,
        0x24, // OP_NEQ
        6,
        6,
        1, 999,
        6
    };
    assert(interop_stack_vm_execute(&vm, script, 14) == 0);
    printf("  [Step 2] VM query blocked on pending status. Backtracked to alternate branch: stack[0] = %d\n", vm.stack[0]);
    fflush(stdout);
    assert(vm.stack[0] == -9999 && vm.stack_len == 2 && vm.stack[1] == 999);

    // 3. Crystallize Answer (Wiping pending status and writing RELATION_TRUE)
    struct timespec s, e;
    clock_gettime(CLOCK_MONOTONIC, &s);
    
    // Compile verified fact, crystallizing the answer
    tsfi_dat *dat2 = tsfi_dat_compile_relation(trie_root, "question/owns_token", "peer_007", "1");
    assert(dat2 != NULL);
    
    clock_gettime(CLOCK_MONOTONIC, &e);
    long long duration_ns = (e.tv_sec - s.tv_sec) * 1000000000LL + (e.tv_nsec - s.tv_nsec);
    printf("  [Step 3] Answer crystallized to DAT disk rails in %lld ns.\n", duration_ns);
    fflush(stdout);

    // 4. Verify lookup resolves to RELATION_TRUE (Strain relieved)
    const char *q2 = tsfi_dat_search(dat2, "question/owns_token/peer_007/1");
    assert(q2 != NULL && strcmp(q2, "RELATION_TRUE") == 0);
    printf("  [Step 4] Verified question resolved to crystallized fact: %s\n", q2);
    fflush(stdout);

    // Cleanup
    tsfi_dat_destroy(dat1);
    tsfi_dat_destroy(dat2);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Question crystallization study completed successfully!\n");
    fflush(stdout);
    return 0;
}
