#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <openssl/sha.h>
#include "tsfi_dat.h"
#include "tsfi_trie.h"

#define STACK_MAX 16
#define HASH_SIZE 32

typedef struct {
    uint8_t data[HASH_SIZE * 3];
    size_t len;
} StackElement;

typedef struct {
    StackElement elements[STACK_MAX];
    int top;
} BtcStack;

extern bool btc_thunk_execute(const uint8_t *script, size_t script_len, BtcStack *stack);

int main(void) {
    printf("=== TSFi Bitcoin Script DAT Transition Verification ===\n");

    // 1. Setup DAT with a simple key mapping
    tsfi_trie_node *trie = tsfi_trie_create_node('\0');
    tsfi_trie_insert(trie, "sh", "SHELL_EXEC");
    tsfi_dat *dat = tsfi_dat_compile(trie);
    assert(dat != NULL);

    // Find expected final state dynamically
    int expected_final_state = 0;
    int state = 0;
    const char *key = "sh";
    while (*key != '\0') {
        int next = dat->base[state] + (unsigned char)*key;
        state = next;
        key++;
    }
    expected_final_state = state;

    // 2. Generate Bitcoin Script for key "sh"
    uint8_t script[512];
    int script_len = tsfi_dat_generate_btc_script(dat, "sh", expected_final_state, script);
    assert(script_len > 0);
    printf("[BTC_DAT] Generated multi-step script for 'sh' (%d bytes)\n", script_len);
    fflush(stdout);

    // 3. Execute script using stack thunk
    BtcStack stack;
    stack.top = -1;

    bool run_ok = btc_thunk_execute(script, script_len, &stack);
    assert(run_ok == true);
    printf("  [PASS] Multi-step script evaluated successfully!\n");
    fflush(stdout);

    // Confirm final state corresponds to boolean true (1)
    if (stack.top >= 0) {
        int final_val = stack.elements[stack.top].data[0];
        printf("[BTC_DAT] Final stack evaluation value: %d (Expected: 1 for True)\n", final_val);
        fflush(stdout);
        assert(final_val == 1);
    }

    // Verify stack contains exactly one element (no intermediate states preserved)
    printf("[BTC_DAT] Final stack depth: %d\n", stack.top + 1);
    fflush(stdout);
    assert(stack.top == 0);

    // Clean up
    tsfi_trie_destroy(trie);
    tsfi_dat_destroy(dat);

    printf("=== ALL BITCOIN SCRIPT DAT TESTS PASSED ===\n");
    return 0;
}
