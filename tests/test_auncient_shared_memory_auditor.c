#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
static void sha256(const void *data, size_t len, uint8_t *out) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(out, &ctx);
}
#pragma GCC diagnostic pop

// Conduction states
typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// 2-3 Merkle Tree Node
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// Shared Memory structure gated by transistors
typedef struct {
    TwoThreeNode *root;
    gate_state_t write_gate;
} shared_dat_t;

#define TAG_ADMIN 0x01
#define TAG_GUEST 0x02

// Helper to create leaf node
static TwoThreeNode* create_leaf(uint32_t key1, const char *val1) {
    TwoThreeNode *node = (TwoThreeNode*)calloc(1, sizeof(TwoThreeNode));
    node->is_leaf = true;
    node->num_keys = 1;
    node->keys[0] = key1;
    strcpy(node->values[0], val1);
    
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
    sha256(temp, len, node->node_hash);
    return node;
}

// Helper to update leaf node hash
static void update_node_hash(TwoThreeNode *node) {
    if (!node) return;
    if (node->is_leaf) {
        uint8_t temp[256];
        int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
        sha256(temp, len, node->node_hash);
    }
}

// Update leaf in the 2-3 tree DAT
static bool store_table_data(TwoThreeNode *node, uint32_t key, const char *new_value) {
    if (!node) return false;
    if (node->is_leaf) {
        if (node->keys[0] == key) {
            strcpy(node->values[0], new_value);
            update_node_hash(node);
            return true;
        }
        return false;
    }
    // Simple 2-node routing traversal
    return store_table_data(node->children[0], key, new_value);
}

// -------------------------------------------------------------
// Shared Memory Auditor Operations
// -------------------------------------------------------------
bool shared_dat_write(shared_dat_t *shm, uint32_t key, const char *val, uint8_t tag) {
    // Write access is physically gated: conducts only if tag == TAG_ADMIN
    shm->write_gate = (tag == TAG_ADMIN) ? CONDUC_STATE : CUTOFF_STATE;

    if (shm->write_gate == CONDUC_STATE) {
        return store_table_data(shm->root, key, val);
    }
    return false; // Decoupled/Blocked by BJT gate
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT CRYPTOGRAPHIC SHARED MEMORY AUDITOR SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize shared DAT tree
    TwoThreeNode *leaf = create_leaf(500, "SHM_DATA_INIT");
    shared_dat_t shm = {
        .root = leaf,
        .write_gate = CUTOFF_STATE
    };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, shm.root->node_hash, HASH_SIZE);

    // 1. Authorized Write (TAG_ADMIN) -> Should change state and root hash
    printf("[TEST] Writing to shared memory with TAG_ADMIN...\n");
    fflush(stdout);
    bool write_ok = shared_dat_write(&shm, 500, "SHM_DATA_UPDATED", TAG_ADMIN);
    assert(write_ok == true);
    assert(strcmp(shm.root->values[0], "SHM_DATA_UPDATED") == 0);
    assert(memcmp(initial_hash, shm.root->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Write completed and root hash updated successfully.\n");
    fflush(stdout);

    // Save updated hash state
    uint8_t updated_hash[HASH_SIZE];
    memcpy(updated_hash, shm.root->node_hash, HASH_SIZE);

    // 2. Unauthorized Write (TAG_GUEST) -> Should be blocked and hash remain constant
    printf("[TEST] Writing to shared memory with TAG_GUEST...\n");
    fflush(stdout);
    write_ok = shared_dat_write(&shm, 500, "TAMPERED_SHM_DATA", TAG_GUEST);
    assert(write_ok == false);
    assert(strcmp(shm.root->values[0], "SHM_DATA_UPDATED") == 0); // Unchanged
    assert(memcmp(updated_hash, shm.root->node_hash, HASH_SIZE) == 0); // Unchanged
    printf("   ✓ Write blocked at transistor gate level.\n");
    fflush(stdout);

    // 3. Simulated direct memory tampering (Bypassing functions) -> Hash must mismatch
    printf("[TEST] Simulating direct memory tampering audit...\n");
    fflush(stdout);
    // Directly mutate buffer without calling hash updates (tampering simulation)
    strcpy(shm.root->values[0], "DIRECT_TAMPER_ATTEMPT");
    
    // Perform verification step: calculate current physical hash and match against expected tree hash
    uint8_t verify_hash[HASH_SIZE];
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", shm.root->keys[0], shm.root->values[0]);
    sha256(temp, len, verify_hash);

    assert(memcmp(verify_hash, shm.root->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Tampering detected: dynamic hash diverges from Merkle registry.\n");
    fflush(stdout);

    free(leaf);
    printf("=============================================================\n");
    printf("SHARED MEMORY AUDITOR INTEGRATION TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
