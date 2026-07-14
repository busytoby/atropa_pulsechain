#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <openssl/sha.h>

#define STACK_MAX 16
#define HASH_SIZE 32

typedef struct {
    uint8_t data[HASH_SIZE * 3]; // Room for three concatenated hashes (96 bytes)
    size_t len;
} StackElement;

typedef struct {
    StackElement elements[STACK_MAX];
    int top; // Index of top element (-1 when empty)
} BtcStack;

static void stack_init(BtcStack *stack) {
    stack->top = -1;
}

static bool stack_push(BtcStack *stack, const uint8_t *data, size_t len) {
    if (stack->top >= STACK_MAX - 1) return false;
    stack->top++;
    memcpy(stack->elements[stack->top].data, data, len);
    stack->elements[stack->top].len = len;
    return true;
}

static bool stack_pop(BtcStack *stack, StackElement *out) {
    if (stack->top < 0) return false;
    *out = stack->elements[stack->top];
    stack->top--;
    return true;
}

// Low-level Bitcoin Script opcode emulator thunk for ZMM VM interop
bool btc_thunk_execute(const uint8_t *script, size_t script_len, BtcStack *stack) {
    size_t pc = 0;
    while (pc < script_len) {
        uint8_t op = script[pc];
        pc++;



        if (op >= 0x01 && op <= 0x4e) {
            // Push data payload onto the stack
            size_t push_len = op;
            if (pc + push_len > script_len) return false;
            if (!stack_push(stack, script + pc, push_len)) return false;
            pc += push_len;
        } else {
            switch (op) {
                case 0xa8: { // OP_SHA256
                    StackElement el;
                    if (!stack_pop(stack, &el)) return false;
                    uint8_t hash[32];
                    SHA256(el.data, el.len, hash);
                    if (!stack_push(stack, hash, 32)) return false;
                    break;
                }
                case 0x7c: { // OP_SWAP
                    if (stack->top < 1) return false;
                    StackElement tmp = stack->elements[stack->top];
                    stack->elements[stack->top] = stack->elements[stack->top - 1];
                    stack->elements[stack->top - 1] = tmp;
                    break;
                }
                case 0x7b: { // OP_ROT
                    if (stack->top < 2) return false;
                    // a b c -> b c a
                    StackElement tmp = stack->elements[stack->top - 2];
                    stack->elements[stack->top - 2] = stack->elements[stack->top - 1];
                    stack->elements[stack->top - 1] = stack->elements[stack->top];
                    stack->elements[stack->top] = tmp;
                    break;
                }
                case 0x7e: { // OP_CAT
                    StackElement right, left;
                    if (!stack_pop(stack, &right)) return false;
                    if (!stack_pop(stack, &left)) return false;
                    
                    size_t new_len = left.len + right.len;
                    if (new_len > HASH_SIZE * 3) return false;
                    
                    uint8_t concat[HASH_SIZE * 3];
                    memcpy(concat, left.data, left.len);
                    memcpy(concat + left.len, right.data, right.len);
                    
                    if (!stack_push(stack, concat, new_len)) return false;
                    break;
                }
                case 0x88: { // OP_EQUALVERIFY
                    StackElement a, b;
                    if (!stack_pop(stack, &a)) return false;
                    if (!stack_pop(stack, &b)) return false;
                    if (a.len != b.len) return false;
                    if (memcmp(a.data, b.data, a.len) != 0) return false;
                    break;
                }
                case 0x87: { // OP_EQUAL
                    StackElement a, b;
                    if (!stack_pop(stack, &a)) return false;
                    if (!stack_pop(stack, &b)) return false;
                    uint8_t res = (a.len == b.len && memcmp(a.data, b.data, a.len) == 0) ? 1 : 0;
                    if (!stack_push(stack, &res, 1)) return false;
                    break;
                }
                case 0x93: { // OP_ADD
                    StackElement a, b;
                    if (!stack_pop(stack, &a)) return false;
                    if (!stack_pop(stack, &b)) return false;
                    
                    int val_a = 0, val_b = 0;
                    if (a.len == 1) val_a = a.data[0];
                    else if (a.len == 4) memcpy(&val_a, a.data, 4);
                    
                    if (b.len == 1) val_b = b.data[0];
                    else if (b.len == 4) memcpy(&val_b, b.data, 4);
                    
                    int sum = val_a + val_b;
                    uint8_t res[4];
                    memcpy(res, &sum, 4);
                    if (!stack_push(stack, res, 4)) return false;
                    break;
                }
                case 0x79: { // OP_PICK
                    StackElement n_el;
                    if (!stack_pop(stack, &n_el)) return false;
                    int n = 0;
                    if (n_el.len == 1) n = n_el.data[0];
                    else if (n_el.len == 4) memcpy(&n, n_el.data, 4);
                    
                    if (stack->top < n) return false;
                    StackElement target = stack->elements[stack->top - n];
                    if (!stack_push(stack, target.data, target.len)) return false;
                    break;
                }
                case 0x7a: { // OP_ROLL
                    StackElement n_el;
                    if (!stack_pop(stack, &n_el)) return false;
                    int n = 0;
                    if (n_el.len == 1) n = n_el.data[0];
                    else if (n_el.len == 4) memcpy(&n, n_el.data, 4);
                    
                    if (stack->top < n) return false;
                    StackElement target = stack->elements[stack->top - n];
                    for (int idx = stack->top - n; idx < stack->top; idx++) {
                        stack->elements[idx] = stack->elements[idx + 1];
                    }
                    stack->elements[stack->top] = target;
                    break;
                }
                default:
                    return false; // Unknown opcode
            }
        }
    }
    return true;
}
