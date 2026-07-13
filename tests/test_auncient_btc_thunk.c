#include "../firmware/tsfi_lib/tsfi_btc_thunk.c"
#include <stdio.h>
#include <assert.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BITCOIN SCRIPT JIT INTEROP THUNK VERIFIER\n");
    printf("=============================================================\n");

    // 1. Reconstruct exact Leaf and Sibling values from our 2-3 Tree test
    uint8_t leaf2_data[] = "Activity 2";
    uint8_t sibling_leaf1[32];
    uint8_t sibling_leaf3[32];
    uint8_t sibling_parent_B[32];

    SHA256((const uint8_t *)"Activity 1", 10, sibling_leaf1);
    SHA256((const uint8_t *)"Activity 3", 10, sibling_leaf3);

    // Compute nodeB sibling
    uint8_t leaf4[32], leaf5[32];
    SHA256((const uint8_t *)"Activity 4", 10, leaf4);
    SHA256((const uint8_t *)"Activity 5", 10, leaf5);
    
    // Hash Node B (leaf4 + leaf5)
    uint8_t concat_B[64];
    memcpy(concat_B, leaf4, 32);
    memcpy(concat_B + 32, leaf5, 32);
    SHA256(concat_B, 64, sibling_parent_B);

    // Expected root hash
    uint8_t expected_root[32];
    // We compute Node A = SHA256(leaf1 + leaf2 + leaf3)
    uint8_t leaf2[32];
    SHA256(leaf2_data, 10, leaf2);
    uint8_t concat_A[96];
    memcpy(concat_A, sibling_leaf1, 32);
    memcpy(concat_A + 32, leaf2, 32);
    memcpy(concat_A + 64, sibling_leaf3, 32);
    uint8_t nodeA[32];
    SHA256(concat_A, 96, nodeA);

    // Root = Node A + Node B
    uint8_t concat_root[64];
    memcpy(concat_root, nodeA, 32);
    memcpy(concat_root + 32, sibling_parent_B, 32);
    SHA256(concat_root, 64, expected_root);

    // 2. Set up the witness stack
    // Stack layout (bottom to top): sibling_parent_B, sibling_leaf3, sibling_leaf1, leaf2_data
    printf("1. Setting up initial witness stack...\n");
    BtcStack stack;
    stack_init(&stack);

    assert(stack_push(&stack, sibling_parent_B, 32));
    assert(stack_push(&stack, sibling_leaf3, 32));
    assert(stack_push(&stack, sibling_leaf1, 32));
    assert(stack_push(&stack, leaf2_data, 10));
    printf("   ✓ Witness stack populated.\n");

    // 3. Assemble the raw Bitcoin Script bytecode
    // Instructions:
    // OP_SHA256 (0xa8)
    // OP_SWAP   (0x7c)
    // OP_ROT    (0x7b)
    // OP_SWAP   (0x7c)
    // OP_ROT    (0x7b)
    // OP_ROT    (0x7b)
    // OP_CAT    (0x7e)
    // OP_SWAP   (0x7c)
    // OP_CAT    (0x7e)
    // OP_SHA256 (0xa8)
    // OP_CAT    (0x7e)
    // OP_SHA256 (0xa8)
    // PUSH_32   (0x20) + 32 bytes expected_root
    // OP_EQUALVERIFY (0x88)
    printf("2. Assembling bytecode for 2-3 Merkle verify...\n");
    uint8_t script[128];
    size_t s_idx = 0;

    script[s_idx++] = 0xa8; // OP_SHA256
    script[s_idx++] = 0x7e; // OP_CAT
    script[s_idx++] = 0x7c; // OP_SWAP
    script[s_idx++] = 0x7e; // OP_CAT
    script[s_idx++] = 0xa8; // OP_SHA256
    script[s_idx++] = 0x7c; // OP_SWAP
    script[s_idx++] = 0x7e; // OP_CAT
    script[s_idx++] = 0xa8; // OP_SHA256
    
    script[s_idx++] = 0x20; // PUSH_32
    memcpy(script + s_idx, expected_root, 32);
    s_idx += 32;

    script[s_idx++] = 0x88; // OP_EQUALVERIFY

    // 4. Run the interpreter thunk
    printf("3. Executing Bitcoin Script on JIT interop thunk...\n");
    bool success = btc_thunk_execute(script, s_idx, &stack);
    assert(success == true);
    printf("   ✓ JIT execution succeeded (Root verified correctly).\n");

    printf("\n=============================================================\n");
    printf("AUNCIENT BITCOIN THUNK VERIFICATION PASSED\n");
    printf("=============================================================\n");
    return 0;
}
