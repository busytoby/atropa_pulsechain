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

// 2-3 Merkle Tree Node for Fourier Registry
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// BASED Fourier Pointer holding target key
typedef struct {
    uint32_t target_key;
} fourier_ptr_t;

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

// -------------------------------------------------------------
// Fourier Registry BASED write dereference
// -------------------------------------------------------------
bool fourier_registry_write(TwoThreeNode *node, const fourier_ptr_t *ptr, 
                            double cosine_bin, double sine_bin, 
                            uint8_t tag, gate_state_t *alu_gate) {
    *alu_gate = CUTOFF_STATE;

    if (!node) return false;

    if (node->is_leaf) {
        if (node->keys[0] == ptr->target_key) {
            // Verify node integrity: recalculate hash and compare
            uint8_t current_verify[HASH_SIZE];
            uint8_t temp[256];
            int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
            sha256(temp, len, current_verify);

            if (memcmp(current_verify, node->node_hash, HASH_SIZE) != 0) {
                return false; // Tampered node
            }

            // Gated conduction opens if tag is valid
            if (tag == TAG_KERNEL) {
                *alu_gate = CONDUC_STATE;
                snprintf(node->values[0], sizeof(node->values[0]), "COS:%.2f,SIN:%.2f", cosine_bin, sine_bin);
                
                int next_len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
                sha256(temp, next_len, node->node_hash);
                return true;
            }
        }
        return false;
    }

    return fourier_registry_write(node->children[0], ptr, cosine_bin, sine_bin, tag, alu_gate);
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT FOURIER REGISTRY VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *root = create_leaf(500, "COS:0.00,SIN:0.00");
    fourier_ptr_t ptr = { .target_key = 500 };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, root->node_hash, HASH_SIZE);
    gate_state_t alu_gate = CUTOFF_STATE;

    // 1. Authorized write (TAG_KERNEL) -> Should update QFT bins and change hash
    printf("[TEST] Writing QFT coefficients (COS: 14.50, SIN: -6.20) with TAG_KERNEL...\n");
    fflush(stdout);
    bool ok = fourier_registry_write(root, &ptr, 14.50, -6.20, TAG_KERNEL, &alu_gate);
    assert(ok == true);
    assert(alu_gate == CONDUC_STATE);
    assert(strcmp(root->values[0], "COS:14.50,SIN:-6.20") == 0);
    assert(memcmp(initial_hash, root->node_hash, HASH_SIZE) != 0);
    printf("   ✓ QFT coefficients saved. Merkle registry evolved.\n");
    fflush(stdout);

    // Save current hash state
    uint8_t updated_hash[HASH_SIZE];
    memcpy(updated_hash, root->node_hash, HASH_SIZE);

    // 2. Unauthorized write (TAG_USER) -> Should block and cutoff
    printf("[TEST] Writing QFT coefficients with TAG_USER...\n");
    fflush(stdout);
    ok = fourier_registry_write(root, &ptr, 99.0, 99.0, TAG_USER, &alu_gate);
    assert(ok == false);
    assert(alu_gate == CUTOFF_STATE); // Cutoff active
    assert(strcmp(root->values[0], "COS:14.50,SIN:-6.20") == 0); // Unaltered
    assert(memcmp(updated_hash, root->node_hash, HASH_SIZE) == 0); // Hash unchanged
    printf("   ✓ Write blocked: authorization check isolated registry node.\n");
    fflush(stdout);

    // 3. Tampering Check: Modify payload directly and verify write fails
    printf("[TEST] Simulating direct registry payload tampering...\n");
    fflush(stdout);
    strcpy(root->values[0], "COS:999.00,SIN:999.00"); // Manual overwrite bypassing gate

    ok = fourier_registry_write(root, &ptr, 10.0, 10.0, TAG_KERNEL, &alu_gate);
    assert(ok == false); // Failed due to hash validation check
    assert(alu_gate == CUTOFF_STATE);
    printf("   ✓ Tampered state identified: write-gate blocked modifications.\n");
    fflush(stdout);

    free(root);
    printf("=============================================================\n");
    printf("FOURIER REGISTRY TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
