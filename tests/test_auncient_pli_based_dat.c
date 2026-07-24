#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
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

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// 2-3 Merkle Tree Node for BASED Storage
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// PL/I BASED pointer holding a 2-3 tree key
typedef struct {
    uint32_t target_key;
} pli_based_dat_ptr_t;

#define TAG_KERNEL 0x01
#define TAG_USER   0x02

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

// Update leaf node hash
static void update_node_hash(TwoThreeNode *node) {
    if (!node) return;
    if (node->is_leaf) {
        uint8_t temp[256];
        int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
        sha256(temp, len, node->node_hash);
    }
}

// -------------------------------------------------------------
// PL/I BASED 2-3 Tree DAT Write dereference
// -------------------------------------------------------------
bool pli_based_dat_write(TwoThreeNode *node, const pli_based_dat_ptr_t *ptr, 
                         const char *new_value, uint8_t tag, gate_state_t *alu_gate) {
    *alu_gate = CUTOFF_STATE;

    // 1. Privilege and traversal check
    if (!node) return false;

    if (node->is_leaf) {
        if (node->keys[0] == ptr->target_key) {
            // Verify node integrity: recalculate hash and compare
            uint8_t current_verify[HASH_SIZE];
            uint8_t temp[256];
            int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
            sha256(temp, len, current_verify);

            if (memcmp(current_verify, node->node_hash, HASH_SIZE) != 0) {
                return false; // Tampered or out-of-sync node state
            }

            // Gated conduction opens if tag is valid
            if (tag == TAG_KERNEL) {
                *alu_gate = CONDUC_STATE;
                strcpy(node->values[0], new_value);
                update_node_hash(node);
                return true;
            }
        }
        return false;
    }

    return pli_based_dat_write(node->children[0], ptr, new_value, tag, alu_gate);
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PL/I BASED 2-3 TREE DAT VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *root = create_leaf(75, "BASED_DAT_PAYLOAD_INIT");
    pli_based_dat_ptr_t ptr = { .target_key = 75 };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, root->node_hash, HASH_SIZE);
    gate_state_t alu_gate = CUTOFF_STATE;

    // 1. Authorized BASED write (TAG_KERNEL) -> Should succeed and update root hash
    printf("[TEST] Writing to BASED pointer key 75 with TAG_KERNEL...\n");
    fflush(stdout);
    bool ok = pli_based_dat_write(root, &ptr, "BASED_DAT_PAYLOAD_LOCKED", TAG_KERNEL, &alu_gate);
    assert(ok == true);
    assert(alu_gate == CONDUC_STATE);
    assert(strcmp(root->values[0], "BASED_DAT_PAYLOAD_LOCKED") == 0);
    assert(memcmp(initial_hash, root->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Write completed successfully. Merkle node updated.\n");
    fflush(stdout);

    // Save current hash state
    uint8_t updated_hash[HASH_SIZE];
    memcpy(updated_hash, root->node_hash, HASH_SIZE);

    // 2. Unauthorized BASED write (TAG_USER) -> Should fail/cutoff
    printf("[TEST] Writing to BASED pointer key 75 with TAG_USER...\n");
    fflush(stdout);
    ok = pli_based_dat_write(root, &ptr, "BASED_DAT_USER_TAMPER", TAG_USER, &alu_gate);
    assert(ok == false);
    assert(alu_gate == CUTOFF_STATE); // Cutoff active
    assert(strcmp(root->values[0], "BASED_DAT_PAYLOAD_LOCKED") == 0); // Unaltered
    assert(memcmp(updated_hash, root->node_hash, HASH_SIZE) == 0); // Hash unchanged
    printf("   ✓ Write blocked: authorization check isolated target node.\n");
    fflush(stdout);

    // 3. Tampering Check: Modify payload directly and verify write fails
    printf("[TEST] Simulating direct node payload tampering...\n");
    fflush(stdout);
    strcpy(root->values[0], "DIRECT_TAMPER_ATTEMPT"); // Direct write bypassing write gate

    ok = pli_based_dat_write(root, &ptr, "OVERWRITE_ATTEMPT", TAG_KERNEL, &alu_gate);
    assert(ok == false); // Failed due to hash validation check
    assert(alu_gate == CUTOFF_STATE);
    printf("   ✓ Tampered state identified: write-gate blocked modifications.\n");
    fflush(stdout);

    free(root);
    printf("=============================================================\n");
    printf("PL/I BASED 2-3 TREE DAT TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
