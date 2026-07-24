#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define NOISE_LIMIT 15.0

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

// 2-3 Merkle Tree Node
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// Improved BASED Fourier Pointer
typedef struct {
    uint32_t target_key;
    uint32_t seq;
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
// Improved Fourier Registry Gated Write
// -------------------------------------------------------------
bool fourier_registry_write_improved(TwoThreeNode *node, fourier_ptr_t *ptr, 
                                     uint32_t expected_seq, double cosine_bin, double sine_bin, 
                                     uint8_t tag, gate_state_t *alu_gate) {
    *alu_gate = CUTOFF_STATE;

    if (!node) return false;

    // 1. Sequence check (Anti-Replay)
    if (ptr->seq != expected_seq) {
        return false;
    }

    // 2. Active Band Gating: Cutoff if QFT coefficients exceed the noise safety threshold
    if (fabs(cosine_bin) > NOISE_LIMIT || fabs(sine_bin) > NOISE_LIMIT) {
        return false; // Isolated: high-frequency noise spike detected
    }

    if (node->is_leaf) {
        if (node->keys[0] == ptr->target_key) {
            // Verify node hash integrity
            uint8_t current_verify[HASH_SIZE];
            uint8_t temp[256];
            int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
            sha256(temp, len, current_verify);

            if (memcmp(current_verify, node->node_hash, HASH_SIZE) != 0) {
                return false;
            }

            // Gated conduction opens if tag matches KERNEL
            if (tag == TAG_KERNEL) {
                *alu_gate = CONDUC_STATE;
                snprintf(node->values[0], sizeof(node->values[0]), "COS:%.2f,SIN:%.2f", cosine_bin, sine_bin);
                
                int next_len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
                sha256(temp, next_len, node->node_hash);

                ptr->seq++; // Advance sequence
                return true;
            }
        }
        return false;
    }

    return fourier_registry_write_improved(node->children[0], ptr, expected_seq, cosine_bin, sine_bin, tag, alu_gate);
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT IMPROVED FOURIER REGISTRY VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *root = create_leaf(600, "COS:0.00,SIN:0.00");
    fourier_ptr_t ptr = { .target_key = 600, .seq = 10 };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, root->node_hash, HASH_SIZE);
    gate_state_t alu_gate = CUTOFF_STATE;

    // 1. Valid write (Within noise limit and correct sequence) -> Should succeed
    printf("[TEST] Writing valid spectral bins (COS: 8.50, SIN: -4.20)...\n");
    fflush(stdout);
    bool ok = fourier_registry_write_improved(root, &ptr, 10, 8.50, -4.20, TAG_KERNEL, &alu_gate);
    assert(ok == true);
    assert(alu_gate == CONDUC_STATE);
    assert(strcmp(root->values[0], "COS:8.50,SIN:-4.20") == 0);
    assert(ptr.seq == 11);
    printf("   ✓ Bins written successfully. Sequence advanced to %u.\n", ptr.seq);
    fflush(stdout);

    // Save hash state
    uint8_t step1_hash[HASH_SIZE];
    memcpy(step1_hash, root->node_hash, HASH_SIZE);

    // 2. Block write due to high-frequency noise spike (COS: 20.00 > 15.0 limit) -> Should cutoff
    printf("[TEST] Writing invalid spectral bins with high-frequency noise spike (COS: 20.00)...\n");
    fflush(stdout);
    ok = fourier_registry_write_improved(root, &ptr, 11, 20.00, -4.20, TAG_KERNEL, &alu_gate);
    assert(ok == false);
    assert(alu_gate == CUTOFF_STATE); // Cutoff active
    assert(strcmp(root->values[0], "COS:8.50,SIN:-4.20") == 0); // Unaltered
    assert(ptr.seq == 11); // Unchanged
    printf("   ✓ High-frequency noise spike blocked successfully by band gate.\n");
    fflush(stdout);

    // 3. Block write due to replayed sequence (Using old sequence 10 instead of 11) -> Should cutoff
    printf("[TEST] Writing replayed spectral sequence (Seq: 10)...\n");
    fflush(stdout);
    ok = fourier_registry_write_improved(root, &ptr, 10, 8.50, -4.20, TAG_KERNEL, &alu_gate);
    assert(ok == false);
    assert(alu_gate == CUTOFF_STATE);
    printf("   ✓ Replayed sequence blocked successfully by sequence check.\n");
    fflush(stdout);

    free(root);
    printf("=============================================================\n");
    printf("IMPROVED FOURIER REGISTRY TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
