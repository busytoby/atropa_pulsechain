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
    printf("=== TSFi ZMM Bitcoin Script Thunk 'ls' Verification ===\n");

    // 1. Setup DAT with the directory mapping
    tsfi_trie_node *trie = tsfi_trie_create_node('\0');
    tsfi_trie_insert(trie, "ls", "DAT_LS_EXEC");
    tsfi_dat *dat = tsfi_dat_compile(trie);
    assert(dat != NULL);

    // Find expected final state dynamically for key "ls"
    int expected_final_state = 0;
    int state = 0;
    const char *key = "ls";
    while (*key != '\0') {
        int next = dat->base[state] + (unsigned char)*key;
        state = next;
        key++;
    }
    expected_final_state = state;

    // 2. Generate Bitcoin Script for path validation phase
    uint8_t script[512];
    int script_len = tsfi_dat_generate_btc_script(dat, "ls", expected_final_state, script);
    assert(script_len > 0);

    // 3. Append execution phase to script
    // We append:
    // - PUSH 1 (1 byte)
    // - OP_EQUALVERIFY (0x88) - asserts the path verification result is true (1)
    // - PUSH "ls /bin" (7 bytes) -> 0x07 [bytes]
    // - PUSH register offset 0 (4 bytes) -> 0x04 [0x00 0x00 0x00 0x00]
    script[script_len++] = 0x01; // PUSH 1 byte
    script[script_len++] = 0x01; // value 1 (true)
    script[script_len++] = 0x88; // OP_EQUALVERIFY

    // Push command payload
    const char *cmd = "ls /bin";
    script[script_len++] = 0x07; // PUSH 7 bytes
    memcpy(script + script_len, cmd, 7);
    script_len += 7;

    // Push target register offset
    int reg_offset = 0;
    script[script_len++] = 0x04; // PUSH 4 bytes
    memcpy(script + script_len, &reg_offset, 4);
    script_len += 4;

    printf("[THUNK] Compiled unified thunk script of length %d bytes.\n", script_len);
    fflush(stdout);

    // 4. Execute script using stack thunk
    BtcStack stack;
    stack.top = -1;

    // Push initial validation true (1) to satisfy OP_EQUALVERIFY for the path phase
    // In actual VM, the path phase leaves 1 on stack, which the appended OP_EQUALVERIFY pops
    bool run_ok = btc_thunk_execute(script, script_len, &stack);
    assert(run_ok == true);
    printf("  [PASS] Thunk script evaluated successfully!\n");
    fflush(stdout);

    // 5. Intercept and verify simulated register write
    assert(stack.top == 1); // Stack has [command, offset]
    
    int resolved_offset = -1;
    memcpy(&resolved_offset, stack.elements[stack.top].data, 4);
    
    char resolved_cmd[16];
    memset(resolved_cmd, 0, sizeof(resolved_cmd));
    memcpy(resolved_cmd, stack.elements[stack.top - 1].data, stack.elements[stack.top - 1].len);

    printf("[THUNK] Target register offset: %d\n", resolved_offset);
    printf("[THUNK] Evaluated command payload: %s\n", resolved_cmd);
    fflush(stdout);

    assert(resolved_offset == 0);
    assert(strcmp(resolved_cmd, "ls /bin") == 0);
    printf("  [PASS] Correct command and offset registers written to ZMM firmware thunk!\n");
    fflush(stdout);

    // Clean up
    tsfi_trie_destroy(trie);
    tsfi_dat_destroy(dat);

    printf("=== ALL ZMM BITCOIN THUNK TESTS PASSED ===\n");
    return 0;
}
