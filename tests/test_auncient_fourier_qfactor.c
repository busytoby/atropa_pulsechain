#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define Q_MIN_LIMIT 0.5

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
// Q-Factor Gated Fourier Registry Write
// -------------------------------------------------------------
bool fourier_registry_write_qfactor(TwoThreeNode *node, const fourier_ptr_t *ptr, 
                                    double cosine_bin, double sine_bin, double noise_floor,
                                    uint8_t tag, gate_state_t *alu_gate) {
    *alu_gate = CUTOFF_STATE;

    if (!node || noise_floor <= 0.0) return false;

    // 1. Calculate Q-Factor: Q = sqrt(cos^2 + sin^2) / noise_floor
    double amplitude = sqrt((cosine_bin * cosine_bin) + (sine_bin * sine_bin));
    double q_factor = amplitude / noise_floor;

    // 2. Q-Factor Gating check: Block write if signal is decoupled/damped (Q < 0.5)
    if (q_factor < Q_MIN_LIMIT) {
        return false; // Halted: Insufficient resonance
    }

    if (node->is_leaf) {
        if (node->keys[0] == ptr->target_key) {
            // Verify hash integrity
            uint8_t current_verify[HASH_SIZE];
            uint8_t temp[256];
            int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
            sha256(temp, len, current_verify);

            if (memcmp(current_verify, node->node_hash, HASH_SIZE) != 0) {
                return false;
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

    return fourier_registry_write_qfactor(node->children[0], ptr, cosine_bin, sine_bin, noise_floor, tag, alu_gate);
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT FOURIER Q-FACTOR GATE VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *root = create_leaf(800, "COS:0.00,SIN:0.00");
    fourier_ptr_t ptr = { .target_key = 800 };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, root->node_hash, HASH_SIZE);
    gate_state_t alu_gate = CUTOFF_STATE;

    // 1. High Q-Factor (COS: 10.00, SIN: 10.00, Noise: 2.0 -> Q = 14.14 / 2 = 7.07 > 0.5) -> Should succeed
    printf("[TEST] Writing high Q-factor signal (Q = 7.07)...\n");
    fflush(stdout);
    bool ok = fourier_registry_write_qfactor(root, &ptr, 10.00, 10.00, 2.0, TAG_KERNEL, &alu_gate);
    assert(ok == true);
    assert(alu_gate == CONDUC_STATE);
    assert(strcmp(root->values[0], "COS:10.00,SIN:10.00") == 0);
    printf("   ✓ Write completed successfully. Q-factor meets threshold.\n");
    fflush(stdout);

    // Save hash state
    uint8_t step1_hash[HASH_SIZE];
    memcpy(step1_hash, root->node_hash, HASH_SIZE);

    // 2. Damped Low Q-Factor (COS: 0.10, SIN: 0.10, Noise: 2.0 -> Q = 0.14 / 2 = 0.07 < 0.5) -> Should cutoff
    printf("[TEST] Writing damped low Q-factor signal (Q = 0.07)...\n");
    fflush(stdout);
    ok = fourier_registry_write_qfactor(root, &ptr, 0.10, 0.10, 2.0, TAG_KERNEL, &alu_gate);
    assert(ok == false);
    assert(alu_gate == CUTOFF_STATE); // Cutoff active
    assert(strcmp(root->values[0], "COS:10.00,SIN:10.00") == 0); // Unaltered
    assert(memcmp(step1_hash, root->node_hash, HASH_SIZE) == 0); // Hash unchanged
    printf("   ✓ Damped signal blocked successfully by Q-factor gate.\n");
    fflush(stdout);

    free(root);
    printf("=============================================================\n");
    printf("FOURIER Q-FACTOR GATE TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
