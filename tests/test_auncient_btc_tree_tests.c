#include "../firmware/tsfi_lib/tsfi_btc_thunk.c"
#include <stdio.h>
#include <assert.h>

// Helper to compute node hash for testing assertions
void get_sha256(const uint8_t *data, size_t len, uint8_t *out) {
    SHA256(data, len, out);
}

// Scenario 1: Standard 2-Node Verification Test
void test_standard_2node_verification(void) {
    printf("  [TEST 1] Running standard 2-node Merkle verification...\n");
    
    uint8_t leaf1[32], leaf2[32], root[32];
    get_sha256((const uint8_t *)"Leaf1", 5, leaf1);
    get_sha256((const uint8_t *)"Leaf2", 5, leaf2);
    
    // Parent Root = leaf1 + leaf2
    uint8_t concat[64];
    memcpy(concat, leaf1, 32);
    memcpy(concat + 32, leaf2, 32);
    get_sha256(concat, 64, root);

    // Witness: sibling_leaf2, leaf1_raw
    BtcStack stack;
    stack_init(&stack);
    assert(stack_push(&stack, leaf2, 32));
    assert(stack_push(&stack, (const uint8_t *)"Leaf1", 5));

    // Script: OP_SHA256 OP_SWAP OP_CAT OP_SHA256 PUSH_32(root) OP_EQUALVERIFY
    uint8_t script[42];
    size_t s_idx = 0;
    script[s_idx++] = 0xa8; // OP_SHA256
    script[s_idx++] = 0x7c; // OP_SWAP
    script[s_idx++] = 0x7e; // OP_CAT
    script[s_idx++] = 0xa8; // OP_SHA256
    script[s_idx++] = 0x20; // PUSH_32
    memcpy(script + s_idx, root, 32);
    s_idx += 32;
    script[s_idx++] = 0x88; // OP_EQUALVERIFY

    bool success = btc_thunk_execute(script, s_idx, &stack);
    assert(success == true);
    printf("    ✓ Standard 2-node verification passed.\n");
}

// Scenario 2: Standard 3-Node Verification Test
void test_standard_3node_verification(void) {
    printf("  [TEST 2] Running standard 3-node Merkle verification...\n");
    
    uint8_t leaf1[32], leaf2[32], leaf3[32], root[32];
    get_sha256((const uint8_t *)"Leaf1", 5, leaf1);
    get_sha256((const uint8_t *)"Leaf2", 5, leaf2);
    get_sha256((const uint8_t *)"Leaf3", 5, leaf3);
    
    // Parent Root = leaf1 + leaf2 + leaf3
    uint8_t concat[96];
    memcpy(concat, leaf1, 32);
    memcpy(concat + 32, leaf2, 32);
    memcpy(concat + 64, leaf3, 32);
    get_sha256(concat, 96, root);

    // Witness: sibling_leaf3, sibling_leaf1, leaf2_raw
    BtcStack stack;
    stack_init(&stack);
    assert(stack_push(&stack, leaf3, 32));
    assert(stack_push(&stack, leaf1, 32));
    assert(stack_push(&stack, (const uint8_t *)"Leaf2", 5));

    // Script: OP_SHA256 OP_CAT OP_SWAP OP_CAT OP_SHA256 PUSH_32(root) OP_EQUALVERIFY
    uint8_t script[42];
    size_t s_idx = 0;
    script[s_idx++] = 0xa8; // OP_SHA256
    script[s_idx++] = 0x7e; // OP_CAT
    script[s_idx++] = 0x7c; // OP_SWAP
    script[s_idx++] = 0x7e; // OP_CAT
    script[s_idx++] = 0xa8; // OP_SHA256
    script[s_idx++] = 0x20; // PUSH_32
    memcpy(script + s_idx, root, 32);
    s_idx += 32;
    script[s_idx++] = 0x88; // OP_EQUALVERIFY

    bool success = btc_thunk_execute(script, s_idx, &stack);
    assert(success == true);
    printf("    ✓ Standard 3-node verification passed.\n");
}

// Scenario 3: Adversarial/Invalid Proof Failure Test
void test_adversarial_failure_detection(void) {
    printf("  [TEST 3] Running adversarial validation tests (expecting verification failures)...\n");
    
    uint8_t leaf1[32], leaf2[32], root[32];
    get_sha256((const uint8_t *)"Leaf1", 5, leaf1);
    get_sha256((const uint8_t *)"Leaf2", 5, leaf2);
    
    uint8_t concat[64];
    memcpy(concat, leaf1, 32);
    memcpy(concat + 32, leaf2, 32);
    get_sha256(concat, 64, root);

    // Case A: Altered leaf data payload (tampered input)
    BtcStack stackA;
    stack_init(&stackA);
    assert(stack_push(&stackA, leaf2, 32));
    assert(stack_push(&stackA, (const uint8_t *)"TAMPERED", 8));

    uint8_t script[42];
    size_t s_idx = 0;
    script[s_idx++] = 0xa8; // OP_SHA256
    script[s_idx++] = 0x7c; // OP_SWAP
    script[s_idx++] = 0x7e; // OP_CAT
    script[s_idx++] = 0xa8; // OP_SHA256
    script[s_idx++] = 0x20; // PUSH_32
    memcpy(script + s_idx, root, 32);
    s_idx += 32;
    script[s_idx++] = 0x88; // OP_EQUALVERIFY

    bool successA = btc_thunk_execute(script, s_idx, &stackA);
    assert(successA == false); // Expect failure
    printf("    ✓ Intercepted tampered leaf payload correctly.\n");

    // Case B: Incorrect sibling hash
    uint8_t wrong_sibling[32];
    memset(wrong_sibling, 0xFF, 32);

    BtcStack stackB;
    stack_init(&stackB);
    assert(stack_push(&stackB, wrong_sibling, 32));
    assert(stack_push(&stackB, (const uint8_t *)"Leaf1", 5));

    bool successB = btc_thunk_execute(script, s_idx, &stackB);
    assert(successB == false); // Expect failure
    printf("    ✓ Intercepted invalid sibling hash correctly.\n");
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ACAB: BITCOIN SCRIPT MERKLE TREE TEST SUITE\n");
    printf("=============================================================\n");

    test_standard_2node_verification();
    test_standard_3node_verification();
    test_adversarial_failure_detection();

    printf("\n=============================================================\n");
    printf("ALL BITCOIN SCRIPT ACAB TREE TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
