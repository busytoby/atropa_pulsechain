#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <openssl/sha.h>

#define LEAF_COUNT 4
#define HASH_SIZE 32

typedef struct {
    uint32_t coordinate_index;
    char activity_type[32];
    uint64_t value;
} ActivityEvent;

// Compute SHA256 of data block
void sha256(const void *data, size_t len, uint8_t *out) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(out, &ctx);
}

// Compute Merkle parent from two child hashes
void compute_parent_hash(const uint8_t *left, const uint8_t *right, uint8_t *parent) {
    uint8_t concat[HASH_SIZE * 2];
    memcpy(concat, left, HASH_SIZE);
    memcpy(concat + HASH_SIZE, right, HASH_SIZE);
    sha256(concat, HASH_SIZE * 2, parent);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COAXIAL ACTIVITY BUS (ACAB): MERKLE COORDINATE COMPILER\n");
    printf("=============================================================\n");

    // 1. Define coordinate-ordered Activity Events (analogous to UTXO transition proofs)
    ActivityEvent events[LEAF_COUNT] = {
        { .coordinate_index = 0, .activity_type = "UTXO_SPENT", .value = 1000 },
        { .coordinate_index = 1, .activity_type = "CHANNEL_DEPOSIT", .value = 5000 },
        { .coordinate_index = 2, .activity_type = "COAXIAL_MODULATION", .value = 440 },
        { .coordinate_index = 3, .activity_type = "STATE_TEARDOWN", .value = 0 }
    };

    printf("1. Compiling coordinate-ordered activity events to leaves...\n");
    uint8_t leaves[LEAF_COUNT][HASH_SIZE];
    for (int i = 0; i < LEAF_COUNT; i++) {
        sha256(&events[i], sizeof(ActivityEvent), leaves[i]);
    }
    printf("   ✓ Leaves hashed successfully.\n");

    // 2. Build Binary Merkle Tree
    printf("2. Building Merkle tree to calculate Activity Root...\n");
    uint8_t parent1[HASH_SIZE];
    uint8_t parent2[HASH_SIZE];
    uint8_t root[HASH_SIZE];

    compute_parent_hash(leaves[0], leaves[1], parent1);
    compute_parent_hash(leaves[2], leaves[3], parent2);
    compute_parent_hash(parent1, parent2, root);

    printf("   ✓ Activity Root Hash: ");
    for (int i = 0; i < HASH_SIZE; i++) printf("%02x", root[i]);
    printf("\n");

    // 3. Generate Merkle Proof for Coordinate Index 2 (COAXIAL_MODULATION)
    // The proof path consists of the sibling at index 3 (leaves[3]) and the parent of indices 0 and 1 (parent1)
    printf("3. Generating Coordinate Activity Proof for Index 2...\n");
    uint8_t proof_step1[HASH_SIZE]; // Sibling: leaves[3]
    uint8_t proof_step2[HASH_SIZE]; // Sibling: parent1
    memcpy(proof_step1, leaves[3], HASH_SIZE);
    memcpy(proof_step2, parent1, HASH_SIZE);

    // 4. Verify Merkle Proof (Simulating Bitcoin Script CLTV / OP_SHA256 logic)
    printf("4. Verifying proof against the published Activity Root...\n");
    uint8_t temp_hash[HASH_SIZE];
    
    // Hash Index 2 Leaf with Sibling at Index 3 (Right side)
    compute_parent_hash(leaves[2], proof_step1, temp_hash);
    
    // Hash result with sibling parent1 (Left side, order: parent1 + temp_hash)
    uint8_t final_hash[HASH_SIZE];
    compute_parent_hash(proof_step2, temp_hash, final_hash);

    // Assert computed root matches published root
    assert(memcmp(final_hash, root, HASH_SIZE) == 0);
    printf("   ✓ Verification successful: Coordinate proof matches published Activity Root!\n");

    printf("\n=============================================================\n");
    printf("AUNCIENT ACTIVITY BUS (ACAB) TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
