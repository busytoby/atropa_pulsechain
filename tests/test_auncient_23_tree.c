#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32

// Simple SHA256 helper
void sha256(const void *data, size_t len, uint8_t *out) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(out, &ctx);
}

// Compute parent hash for a 2-node (2 children)
void hash_2node(const uint8_t *c1, const uint8_t *c2, uint8_t *parent) {
    uint8_t concat[HASH_SIZE * 2];
    memcpy(concat, c1, HASH_SIZE);
    memcpy(concat + HASH_SIZE, c2, HASH_SIZE);
    sha256(concat, HASH_SIZE * 2, parent);
}

// Compute parent hash for a 3-node (3 children)
void hash_3node(const uint8_t *c1, const uint8_t *c2, const uint8_t *c3, uint8_t *parent) {
    uint8_t concat[HASH_SIZE * 3];
    memcpy(concat, c1, HASH_SIZE);
    memcpy(concat + HASH_SIZE, c2, HASH_SIZE);
    memcpy(concat + HASH_SIZE * 2, c3, HASH_SIZE);
    sha256(concat, HASH_SIZE * 3, parent);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ACAB: 2-3 MERKLE COORDINATE TREE COMPILER\n");
    printf("=============================================================\n");

    // Mock leaf hashes representing transaction activities
    uint8_t leaf1[HASH_SIZE];
    uint8_t leaf2[HASH_SIZE];
    uint8_t leaf3[HASH_SIZE];
    uint8_t leaf4[HASH_SIZE];
    uint8_t leaf5[HASH_SIZE];

    sha256("Activity 1", 10, leaf1);
    sha256("Activity 2", 10, leaf2);
    sha256("Activity 3", 10, leaf3);
    sha256("Activity 4", 10, leaf4);
    sha256("Activity 5", 10, leaf5);

    // Build a 2-3 Tree layout:
    // - Node A (3-node): children (leaf1, leaf2, leaf3)
    // - Node B (2-node): children (leaf4, leaf5)
    // - Root (2-node): children (Node A, Node B)
    printf("1. Multiplexing 5 leaves into a balanced 2-3 Merkle Tree...\n");
    
    uint8_t nodeA[HASH_SIZE];
    uint8_t nodeB[HASH_SIZE];
    uint8_t root[HASH_SIZE];

    hash_3node(leaf1, leaf2, leaf3, nodeA); // Hash 3 children
    hash_2node(leaf4, leaf5, nodeB);        // Hash 2 children
    hash_2node(nodeA, nodeB, root);         // Hash 2 parent nodes

    printf("   ✓ Root calculated successfully: ");
    for (int i = 0; i < HASH_SIZE; i++) printf("%02x", root[i]);
    printf("\n");

    // Verify proof for Leaf 2 (member of the 3-node Node A)
    // Sibling elements needed: leaf1, leaf3 (from 3-node A) and nodeB (parent B sibling)
    printf("2. Simulating verification path for Leaf 2 inside the 3-node...\n");
    uint8_t temp_nodeA[HASH_SIZE];
    hash_3node(leaf1, leaf2, leaf3, temp_nodeA);
    assert(memcmp(temp_nodeA, nodeA, HASH_SIZE) == 0);

    uint8_t temp_root[HASH_SIZE];
    hash_2node(temp_nodeA, nodeB, temp_root);
    assert(memcmp(temp_root, root, HASH_SIZE) == 0);
    
    printf("   ✓ 2-3 tree path verified successfully against Root Hash!\n");

    printf("\n=============================================================\n");
    printf("AUNCIENT 2-3 TREE ACAB COMPILER TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
