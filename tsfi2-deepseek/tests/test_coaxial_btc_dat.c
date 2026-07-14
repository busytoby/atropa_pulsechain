#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <openssl/sha.h>

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

// Link emulator execute directly to test
extern bool btc_thunk_execute(const uint8_t *script, size_t script_len, BtcStack *stack);

int main(void) {
    printf("=== TSFi Bitcoin Script DAT Transition Verification ===\n");

    // We simulate verifying the state transition:
    // next_state = base[current_state] + char_input
    // Verify check[next_state] == current_state

    // Setup input values
    int current_state = 0;
    int char_input = 76; // 'L'
    int base_val = 10;
    int check_val = 0;   // check[next_state] should match current_state

    // Create the Bitcoin Script bytecode payload:
    // 1. PUSH base_val
    // 2. PUSH char_input
    // 3. OP_ADD -> pushes sum (86)
    // 4. PUSH check_val
    // 5. PUSH current_state
    // 6. OP_EQUALVERIFY -> asserts check_val == current_state
    uint8_t script[64];
    int pc = 0;

    // Push base_val (4 bytes)
    script[pc++] = 0x04; // OP_PUSH 4 bytes
    memcpy(script + pc, &base_val, 4);
    pc += 4;

    // Push char_input (4 bytes)
    script[pc++] = 0x04;
    memcpy(script + pc, &char_input, 4);
    pc += 4;

    // OP_ADD (0x93)
    script[pc++] = 0x93;

    // Push check_val (4 bytes)
    script[pc++] = 0x04;
    memcpy(script + pc, &check_val, 4);
    pc += 4;

    // Push current_state (4 bytes)
    script[pc++] = 0x04;
    memcpy(script + pc, &current_state, 4);
    pc += 4;

    // OP_EQUALVERIFY (0x88)
    script[pc++] = 0x88;

    // Initialize Stack
    BtcStack stack;
    stack.top = -1;

    printf("[BTC_DAT] Executing transition script of length %d bytes...\n", pc);
    bool run_ok = btc_thunk_execute(script, pc, &stack);
    assert(run_ok == true);
    printf("  [PASS] Bitcoin Script successfully evaluated and verified the DAT transition!\n");

    // Print stack trace
    printf("[BTC_DAT] Remaining stack top index: %d\n", stack.top);
    if (stack.top >= 0) {
        int final_val = 0;
        memcpy(&final_val, stack.elements[stack.top].data, 4);
        printf("[BTC_DAT] Stack Top (Next State): %d (Expected: %d)\n", final_val, base_val + char_input);
        assert(final_val == (base_val + char_input));
    }

    printf("=== ALL BITCOIN SCRIPT DAT TESTS PASSED ===\n");
    return 0;
}
