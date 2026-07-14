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

// Helper to resolve the final state index for a key in a DAT
static int get_dat_final_state(tsfi_dat *dat, const char *key) {
    int state = 0;
    while (*key != '\0') {
        int next = dat->base[state] + (unsigned char)*key;
        state = next;
        key++;
    }
    return state;
}

// Helper to verify a thunk command execution
static void verify_thunk_cmd(tsfi_dat *dat, const char *key, const char *expected_cmd) {
    int expected_state = get_dat_final_state(dat, key);
    assert(expected_state > 0);

    uint8_t script[512];
    int script_len = tsfi_dat_generate_btc_script(dat, key, expected_state, script);
    assert(script_len > 0);

    // Append OP_EQUALVERIFY and the target command strobe
    script[script_len++] = 0x01; // PUSH 1 byte
    script[script_len++] = 0x01; // value 1 (true)
    script[script_len++] = 0x88; // OP_EQUALVERIFY

    // Push command
    size_t cmd_len = strlen(expected_cmd);
    script[script_len++] = (uint8_t)cmd_len;
    memcpy(script + script_len, expected_cmd, cmd_len);
    script_len += cmd_len;

    // Push register offset 0
    int offset = 0;
    script[script_len++] = 0x04;
    memcpy(script + script_len, &offset, 4);
    script_len += 4;

    BtcStack stack;
    stack.top = -1;

    bool run_ok = btc_thunk_execute(script, script_len, &stack);
    assert(run_ok == true);

    // Verify results
    assert(stack.top == 1);
    int resolved_offset = -1;
    memcpy(&resolved_offset, stack.elements[stack.top].data, 4);
    assert(resolved_offset == 0);

    char resolved_cmd[64];
    memset(resolved_cmd, 0, sizeof(resolved_cmd));
    memcpy(resolved_cmd, stack.elements[stack.top - 1].data, stack.elements[stack.top - 1].len);
    assert(strcmp(resolved_cmd, expected_cmd) == 0);

    printf("  [PASS] Verified thunk for command '%s' -> resolved: '%s' offset: %d\n", key, resolved_cmd, resolved_offset);
    fflush(stdout);
}

int main(void) {
    printf("=== TSFi Bitcoin VM Resolver Comprehensive Test ===\n");

    // 1. Setup unified DAT database
    tsfi_trie_node *trie = tsfi_trie_create_node('\0');
    tsfi_trie_insert(trie, "ls", "DAT_LS_EXEC");
    tsfi_trie_insert(trie, "cat", "DAT_CAT_EXEC");
    tsfi_trie_insert(trie, "echo", "DAT_ECHO_EXEC");
    tsfi_dat *dat = tsfi_dat_compile(trie);
    assert(dat != NULL);

    // 2. Verify each command in sequence
    printf("[RESOLVER] Running sequentially verified execution thunks...\n");
    verify_thunk_cmd(dat, "ls", "ls /bin");
    verify_thunk_cmd(dat, "cat", "cat /etc/motd");
    verify_thunk_cmd(dat, "echo", "echo hello");

    // Clean up
    tsfi_trie_destroy(trie);
    tsfi_dat_destroy(dat);

    printf("=== ALL COMPREHENSIVE RESOLVER TESTS PASSED ===\n");
    return 0;
}
