#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "libantigravity_interop.h"

int main(void) {
    printf("[Auncient BTC Tree] Starting 2-stack BTC rails verification tests...\n");

    // 1. Verify Yul opcode instruction translation
    printf("       [Verify] Yul opcode translation...\n");
    int translated_op = 0;
    assert(interop_yul_translate_opcode(100, &translated_op) == 0); // PUSH
    assert(translated_op == 1);
    assert(interop_yul_translate_opcode(101, &translated_op) == 0); // ADD
    assert(translated_op == 2);
    assert(interop_yul_translate_opcode(102, &translated_op) == 0); // SUB
    assert(translated_op == 3);
    assert(interop_yul_translate_opcode(105, &translated_op) == 0); // HALT/REVERT
    assert(translated_op == 6);
    
    // 2. Verify Yul object execution and memory mapping on 2-stack VM
    printf("       [Verify] Yul object execution and memory mapping...\n");
    InteropStackVM vm;
    memset(&vm, 0, sizeof(InteropStackVM));
    // Bytecode instructions: PUSH 100, PUSH 500, MSTORE, PUSH 200, PUSH 700, MSTORE, HALT
    int yul_bc[11] = { 100, 100, 100, 500, 106, 100, 200, 100, 700, 106, 105 };
    int memory_pages[10] = {0};
    size_t mem_count = 0;
    assert(interop_yul_execute_object(&vm, yul_bc, 11, memory_pages, &mem_count, 5) == 0);
    assert(mem_count == 2);
    assert(memory_pages[0] == 100 && memory_pages[1] == 500);
    assert(memory_pages[2] == 200 && memory_pages[3] == 700);
    
    // 3. Verify memory state validations
    printf("       [Verify] Yul EVM memory proofs...\n");
    int pl_verified = -1;
    assert(interop_yul_verify_memory(memory_pages, mem_count, 100, 500, &pl_verified) == 0);
    assert(pl_verified == 1);
    assert(interop_yul_verify_memory(memory_pages, mem_count, 200, 700, &pl_verified) == 0);
    assert(pl_verified == 1);
    assert(interop_yul_verify_memory(memory_pages, mem_count, 100, 999, &pl_verified) == 0);
    assert(pl_verified == 0);
    
    // 4. Verify recursive VM stack execution
    printf("       [Verify] Recursive VM execution boundaries...\n");
    InteropNestedVM parent, child;
    memset(&parent, 0, sizeof(InteropNestedVM));
    memset(&child, 0, sizeof(InteropNestedVM));
    parent.depth = 2;
    parent.child = &child;
    child.depth = 1;
    // Recursion bytecode payload
    int recursive_bc[9] = { 1, 1, 1, 10, 1, 20, 2, 6 };
    assert(interop_vm_recursive_execute(&parent, recursive_bc, 8) == 0);
    assert(parent.vm.stack_len == 2 && parent.vm.stack[1] == 30);
    assert(child.vm.stack_len == 1 && child.vm.stack[0] == 30);
    
    // 5. Verify recursive state verification depth
    int rec_verified = -1;
    int exp_child_stack[1] = { 30 };
    assert(interop_vm_recursive_verify(&parent, 1, exp_child_stack, 1, &rec_verified) == 0);
    assert(rec_verified == 1);

    printf("[PASS] All 2-stack BTC rails verification tests passed successfully.\n");
    return 0;
}
