#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "../../scripts/libantigravity_interop.h"
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "../../scripts/libantigravity_extra2.c"

int main(void) {
    printf("[Auncient BTC Tree] Starting 2-stack BTC rails verification tests...\n");
    fflush(stdout);

    // 1. Verify Yul opcode instruction translation
    printf("       [Verify] Yul opcode translation...\n");
    fflush(stdout);
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
    fflush(stdout);
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
    fflush(stdout);
    int pl_verified = -1;
    assert(interop_yul_verify_memory(memory_pages, mem_count, 100, 500, &pl_verified) == 0);
    assert(pl_verified == 1);
    assert(interop_yul_verify_memory(memory_pages, mem_count, 200, 700, &pl_verified) == 0);
    assert(pl_verified == 1);
    assert(interop_yul_verify_memory(memory_pages, mem_count, 100, 999, &pl_verified) == 0);
    assert(pl_verified == 0);
    
    // 4. Verify recursive VM stack execution
    printf("       [Verify] Recursive VM execution boundaries...\n");
    fflush(stdout);
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

    // 6. Verify Double-Array Trie (DAT) quadtree persistent binary serialization
    printf("       [Verify] Double-Array Trie (DAT) serialization persistence...\n");
    fflush(stdout);
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_trie_insert(trie_root, "dynamic_0xf39fd6e51aad88f6f4ce6ab8827279cfffb92266/receiver", "receiver_wallet");
    tsfi_trie_insert(trie_root, "dynamic_0xf39fd6e51aad88f6f4ce6ab8827279cfffb92266/operator", "operator_wallet");
    
    tsfi_dat *dat = tsfi_dat_compile(trie_root);
    assert(dat != NULL);
    
    const char *resolved_val = tsfi_dat_search(dat, "dynamic_0xf39fd6e51aad88f6f4ce6ab8827279cfffb92266/receiver");
    assert(resolved_val != NULL && strcmp(resolved_val, "receiver_wallet") == 0);
    
    const char *test_bin_path = "tmp/test_unified_addr.dat.bin";
    assert(tsfi_dat_save_bin(dat, test_bin_path) == 0);
    
    tsfi_dat *loaded_dat = tsfi_dat_load_bin(test_bin_path);
    assert(loaded_dat != NULL);
    const char *loaded_val = tsfi_dat_search(loaded_dat, "dynamic_0xf39fd6e51aad88f6f4ce6ab8827279cfffb92266/receiver");
    assert(loaded_val != NULL && strcmp(loaded_val, "receiver_wallet") == 0);
    
    tsfi_dat_destroy(dat);
    tsfi_dat_destroy(loaded_dat);
    tsfi_trie_destroy(trie_root);

    // 7. Verify Unified 2-3 Tree over DAT on BTC Rails
    printf("       [Verify] Unified 2-3 Tree structural splits over DAT...\n");
    fflush(stdout);
    tsfi_trie_node *tree_root = tsfi_trie_create_node(0);
    
    tsfi_trie_insert(tree_root, "tree_root/type", "2-node");
    tsfi_trie_insert(tree_root, "tree_root/val1", "50");
    tsfi_trie_insert(tree_root, "tree_root/left", "tree_node_a");
    tsfi_trie_insert(tree_root, "tree_root/right", "tree_node_b");
    
    tsfi_trie_insert(tree_root, "tree_node_a/type", "3-node");
    tsfi_trie_insert(tree_root, "tree_node_a/val1", "20");
    tsfi_trie_insert(tree_root, "tree_node_a/val2", "30");
    
    tsfi_trie_insert(tree_root, "tree_node_b/type", "2-node");
    tsfi_trie_insert(tree_root, "tree_node_b/val1", "80");
    
    tsfi_dat *tree_dat = tsfi_dat_compile(tree_root);
    assert(tree_dat != NULL);
    
    const char *node_type = tsfi_dat_search(tree_dat, "tree_root/type");
    assert(node_type != NULL && strcmp(node_type, "2-node") == 0);
    
    const char *node_val = tsfi_dat_search(tree_dat, "tree_node_a/val2");
    assert(node_val != NULL && strcmp(node_val, "30") == 0);
    
    printf("       [Verify] BTC Rails execution proof matching tree constraints...\n");
    fflush(stdout);
    InteropStackVM tree_vm;
    memset(&tree_vm, 0, sizeof(InteropStackVM));
    int btc_tree_script[10] = { 1, 30, 1, 20, 2, 1, 50, 6 };
    assert(interop_stack_vm_execute(&tree_vm, btc_tree_script, 8) == 0);
    assert(tree_vm.stack_len == 1 && tree_vm.stack[0] == 50);
    
    tsfi_dat_destroy(tree_dat);
    tsfi_trie_destroy(tree_root);

    printf("[PASS] All 2-stack BTC rails verification tests passed successfully.\n");
    fflush(stdout);
    return 0;
}
