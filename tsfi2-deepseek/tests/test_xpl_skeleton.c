#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define MAX_STACK_DEPTH 128

// Mock XPL compiler grammar reduction constants
#define REDUCTION_EXPRESSION   100
#define REDUCTION_VCE_ASSIGN   101

// Token types for the SKELETON parser
typedef enum {
    T_BYTE,
    T_LPAREN,
    T_NUMBER,
    T_RPAREN,
    T_ASSIGN,
    T_SEMICOLON,
    T_EXPR
} Token;

// Mock SKELETON Parse Stack
typedef struct {
    Token tokens[MAX_STACK_DEPTH];
    unsigned int values[MAX_STACK_DEPTH];
    int sp; // Stack pointer
} SkeletonStack;

void stack_push(SkeletonStack *stack, Token tok, unsigned int val) {
    assert(stack->sp < MAX_STACK_DEPTH - 1);
    stack->sp++;
    stack->tokens[stack->sp] = tok;
    stack->values[stack->sp] = val;
}

// Emulates SKELETON reduction check: BYTE ( NUMBER ) = NUMBER ;
int skeleton_try_reduce(SkeletonStack *stack, TsfiZmmVmState *vm) {
    int sp = stack->sp;
    
    // Check if the top of the stack matches: BYTE ( NUMBER ) = NUMBER ;
    // Indices: sp-5: BYTE, sp-4: (, sp-3: NUMBER, sp-2: ), sp-1: =, sp: NUMBER
    if (sp >= 5 &&
        stack->tokens[sp - 5] == T_BYTE &&
        stack->tokens[sp - 4] == T_LPAREN &&
        stack->tokens[sp - 3] == T_NUMBER &&
        stack->tokens[sp - 2] == T_RPAREN &&
        stack->tokens[sp - 1] == T_ASSIGN &&
        stack->tokens[sp] == T_NUMBER) {
        
        unsigned int address = stack->values[sp - 3];
        unsigned int value = stack->values[sp];
        
        printf("[SKELETON] Match found on stack. Triggering REDUCTION_VCE_ASSIGN.\n");
        printf("[SKELETON] Semantic action: writing color %u to VCE address %u\n", value, address);
        
        // Execute the VCE register write command (vceWriteColor selector: 0xc14ab90d)
        if (address >= 62208 && address <= 62719) {
            unsigned int color_idx = address - 62208;
            unsigned int r = (value >> 6) & 0x7;
            unsigned int g = (value >> 3) & 0x7;
            unsigned int b = value & 0x7;
            
            char calldata[512];
            sprintf(calldata, "YULEXEC \"huc\", \"c14ab90d"
                              "%064x"
                              "%064x"
                              "%064x"
                              "%064x\"",
                    color_idx, r, g, b);
            tsfi_zmm_vm_exec(vm, calldata);
        }
        
        // Perform the shift-reduce swap on the stack
        stack->sp -= 5; // collapse the 6 tokens
        stack->tokens[stack->sp] = T_EXPR; // replace with expression token
        stack->values[stack->sp] = value;
        return REDUCTION_VCE_ASSIGN;
    }
    
    return 0; // No reduction found
}

int main() {
    printf("=== TSFi ZMM VM Auncient XPL SKELETON Stack Reduction Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and deploy hucSystem.yul
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    // 2. Setup Parse Stack
    SkeletonStack stack;
    stack.sp = -1;

    // Simulate shifting tokens for statement: BYTE(62208) = 146;
    printf("[SKELETON] Simulating shifts for statement: BYTE(62208) = 146;\n");
    stack_push(&stack, T_BYTE, 0);
    stack_push(&stack, T_LPAREN, 0);
    stack_push(&stack, T_NUMBER, 62208);
    stack_push(&stack, T_RPAREN, 0);
    stack_push(&stack, T_ASSIGN, 0);
    stack_push(&stack, T_NUMBER, 146); // (2 << 6) | (2 << 3) | 2 = 128 + 16 + 2 = 146 (0x092)

    // 3. Trigger reduction
    int reduction = skeleton_try_reduce(&stack, &vm);
    assert(reduction == REDUCTION_VCE_ASSIGN);
    assert(stack.sp == 0);
    assert(stack.tokens[0] == T_EXPR);

    // 4. Verify VCE register index 0 holds 146 (0x92) via vceReadColor
    printf("[ZMM] Querying VCE Color Index 0 via vceReadColor...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"huc\", \"22bbfa840000000000000000000000000000000000000000000000000000000000000000\"");
    printf("[ZMM] vceReadColor(0) returned: %s\n", vm.output_buffer);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000092") != NULL);

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL SKELETON STACK REDUCTION TESTS PASSED ===\n");
    return 0;
}
