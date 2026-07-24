#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define STACK_CAPACITY 64

typedef struct {
    int64_t data[STACK_CAPACITY];
    int top;

    // Checkpoint registers for transaction rollbacks
    int64_t checkpoint_data[STACK_CAPACITY];
    int checkpoint_top;
} vppd_stack_t;

static void push_stack(vppd_stack_t *s, int64_t val) {
    if (s->top < STACK_CAPACITY) {
        s->data[s->top++] = val;
    }
}

// Saves current stack state to the checkpoint registers
static void vppd_stack_save_checkpoint(vppd_stack_t *s) {
    s->checkpoint_top = s->top;
    memcpy(s->checkpoint_data, s->data, sizeof(int64_t) * s->top);
}

// Restores stack state to the saved checkpoint
static void vppd_stack_rollback(vppd_stack_t *s) {
    s->top = s->checkpoint_top;
    memcpy(s->data, s->checkpoint_data, sizeof(int64_t) * s->checkpoint_top);
    printf("   [ROLLBACK] Stack state rolled back to checkpoint. Restored top: %d\n", s->top);
}

// Commits the transaction changes
static void vppd_stack_commit(vppd_stack_t *s) {
    s->checkpoint_top = s->top; // Advance checkpoint to current top
    printf("   [COMMIT] Stack transaction committed. Current top: %d\n", s->top);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT VPPD TRANSACTIONAL STACK ROLLBACK SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    vppd_stack_t stack;
    memset(&stack, 0, sizeof(vppd_stack_t));

    // Initial state: push baseline coordinates
    push_stack(&stack, 10);
    push_stack(&stack, 20);
    assert(stack.top == 2);
    printf("[TEST] Initial stack elements pushed (top = 2).\n");
    fflush(stdout);

    // 1. Start Transaction 1 (Failing Path)
    vppd_stack_save_checkpoint(&stack);
    printf("[TEST] Checkpoint saved. Beginning Transaction 1 (expected to fail)...\n");
    fflush(stdout);

    push_stack(&stack, 30);
    push_stack(&stack, 40);
    assert(stack.top == 4);
    assert(stack.data[2] == 30);
    assert(stack.data[3] == 40);

    // Simulate validation/purity constraint failure
    bool transaction_valid = false;
    if (!transaction_valid) {
        vppd_stack_rollback(&stack);
    }

    // Verify rollback
    assert(stack.top == 2);
    assert(stack.data[0] == 10);
    assert(stack.data[1] == 20);
    printf("   ✓ Transaction 1 successfully rolled back. Baseline stack intact.\n");
    fflush(stdout);

    // 2. Start Transaction 2 (Passing Path)
    vppd_stack_save_checkpoint(&stack);
    printf("[TEST] Checkpoint saved. Beginning Transaction 2 (expected to pass)...\n");
    fflush(stdout);

    push_stack(&stack, 50);
    assert(stack.top == 3);
    assert(stack.data[2] == 50);

    // Commit changes
    vppd_stack_commit(&stack);

    // Verify commit
    assert(stack.top == 3);
    assert(stack.data[0] == 10);
    assert(stack.data[1] == 20);
    assert(stack.data[2] == 50);
    printf("   ✓ Transaction 2 successfully committed. Stack updated to top = 3.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("STACK ROLLBACK VERIFICATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
