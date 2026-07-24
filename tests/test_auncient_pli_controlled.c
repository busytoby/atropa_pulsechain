#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

#define STACK_LIMIT 3

// PL/I CONTROLLED Variable Stack
typedef struct {
    uint32_t values[STACK_LIMIT];
    uint32_t sp; // Stack Pointer
    uint32_t limit;
    gate_state_t write_gate;
} pli_controlled_stack_t;

// -------------------------------------------------------------
// PL/I CONTROLLED Allocation and Free Routines
// -------------------------------------------------------------
bool pli_allocate_controlled(pli_controlled_stack_t *stack, uint32_t val) {
    // Wortman Bounds Check: Verify allocation doesn't exceed stack limit
    if (stack->sp >= stack->limit) {
        stack->write_gate = CUTOFF_STATE;
        return false; // Stack Overflow: Allocation blocked
    }

    // Push new allocation instance to stack
    stack->write_gate = CONDUC_STATE;
    stack->values[stack->sp] = val;
    stack->sp++;
    return true;
}

bool pli_free_controlled(pli_controlled_stack_t *stack, uint32_t *popped_val) {
    // Wortman Bounds Check: Verify stack isn't empty
    if (stack->sp == 0) {
        stack->write_gate = CUTOFF_STATE;
        return false; // Stack Underflow: Free blocked
    }

    // Pop active instance and restore prior state
    stack->write_gate = CONDUC_STATE;
    stack->sp--;
    *popped_val = stack->values[stack->sp];
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PL/I CONTROLLED STACK VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    pli_controlled_stack_t stack = {
        .values = {0},
        .sp = 0,
        .limit = STACK_LIMIT,
        .write_gate = CUTOFF_STATE
    };

    // 1. Allocate initial instance (Value = 100) -> Should succeed
    printf("[TEST] Allocating CONTROLLED instance (val = 100)...\n");
    fflush(stdout);
    bool ok = pli_allocate_controlled(&stack, 100);
    assert(ok == true);
    assert(stack.write_gate == CONDUC_STATE);
    assert(stack.sp == 1);
    assert(stack.values[0] == 100);
    printf("   ✓ First allocation pushed successfully.\n");
    fflush(stdout);

    // 2. Allocate shadowed instances up to limit (100 -> 200 -> 300)
    printf("[TEST] Pushing allocations up to stack limit...\n");
    fflush(stdout);
    ok = pli_allocate_controlled(&stack, 200);
    assert(ok == true);
    ok = pli_allocate_controlled(&stack, 300);
    assert(ok == true);
    assert(stack.sp == 3);
    printf("   ✓ Stack filled to limit successfully.\n");
    fflush(stdout);

    // 3. Overflow Check: Attempt fourth allocation (Exceeding STACK_LIMIT = 3) -> Should block/cutoff
    printf("[TEST] Triggering stack overflow allocation attempt...\n");
    fflush(stdout);
    ok = pli_allocate_controlled(&stack, 400);
    assert(ok == false);
    assert(stack.write_gate == CUTOFF_STATE); // Gated/Cutoff active
    assert(stack.sp == 3); // Unchanged
    printf("   ✓ Stack overflow caught: allocation blocked at boundary.\n");
    fflush(stdout);

    // 4. Free instances to restore state
    printf("[TEST] Popping and freeing CONTROLLED allocations...\n");
    fflush(stdout);
    uint32_t val = 0;
    
    ok = pli_free_controlled(&stack, &val);
    assert(ok == true);
    assert(val == 300); // Pops last pushed
    
    ok = pli_free_controlled(&stack, &val);
    assert(ok == true);
    assert(val == 200);

    ok = pli_free_controlled(&stack, &val);
    assert(ok == true);
    assert(val == 100);
    
    assert(stack.sp == 0);
    printf("   ✓ All allocations popped successfully. Prior state restored.\n");
    fflush(stdout);

    // 5. Underflow Check: Free empty stack -> Should block/cutoff
    printf("[TEST] Triggering stack underflow free attempt...\n");
    fflush(stdout);
    ok = pli_free_controlled(&stack, &val);
    assert(ok == false);
    assert(stack.write_gate == CUTOFF_STATE);
    printf("   ✓ Stack underflow caught: operation blocked.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("PL/I CONTROLLED STACK TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
