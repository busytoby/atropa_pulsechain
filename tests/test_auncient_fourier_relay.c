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

// 2-3 Merkle Tree Node for Fourier Registry Slices
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// Fourier Node state representation
typedef struct {
    uint32_t node_id;
    uint32_t target_key;
    double cosine_bin;
    double sine_bin;
    uint32_t seq;
    TwoThreeNode *dat;
} fourier_node_t;

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

// Update leaf node registry value
static void update_registry(TwoThreeNode *node, double cosine, double sine) {
    if (!node) return;
    snprintf(node->values[0], sizeof(node->values[0]), "COS:%.2f,SIN:%.2f", cosine, sine);
    
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
    sha256(temp, len, node->node_hash);
}

// -------------------------------------------------------------
// Fourier Node Relay Transmission Loop
// -------------------------------------------------------------
bool fourier_relay_transmit(fourier_node_t *sender, fourier_node_t *receiver, 
                            uint8_t tag, gate_state_t *relay_gate) {
    *relay_gate = CUTOFF_STATE;

    // 1. Privilege tag check
    if (tag != TAG_KERNEL) return false;

    // 2. Sequence-Interlocked check (Replay protection at receiver side)
    // The receiver expects the sender's sequence to match its own active track
    if (sender->seq != receiver->seq) {
        return false; // Halted: out-of-order relay sequence
    }

    // 3. Conduction opens: transmit spectral coefficients and advance sequence
    *relay_gate = CONDUC_STATE;
    receiver->cosine_bin = sender->cosine_bin;
    receiver->sine_bin = sender->sine_bin;

    // Save update to receiver's BASED 2-3 tree DAT registry
    update_registry(receiver->dat, receiver->cosine_bin, receiver->sine_bin);

    // Sync sequences
    sender->seq++;
    receiver->seq++;

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT FOURIER NODE RELAY NETWORK VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *dat_a = create_leaf(500, "COS:0.00,SIN:0.00");
    TwoThreeNode *dat_b = create_leaf(600, "COS:0.00,SIN:0.00");

    fourier_node_t node_a = {
        .node_id = 1,
        .target_key = 500,
        .cosine_bin = 12.50,
        .sine_bin = -4.50,
        .seq = 10,
        .dat = dat_a
    };

    fourier_node_t node_b = {
        .node_id = 2,
        .target_key = 600,
        .cosine_bin = 0.0,
        .sine_bin = 0.0,
        .seq = 10, // Initial expected sequence matching node_a
        .dat = dat_b
    };

    uint8_t initial_hash_b[HASH_SIZE];
    memcpy(initial_hash_b, node_b.dat->node_hash, HASH_SIZE);
    gate_state_t relay_gate = CUTOFF_STATE;

    // 1. Authorized Relay (TAG_KERNEL, matching sequence 10) -> Should succeed
    printf("[TEST] Relaying QFT coefficients from Node A to Node B...\n");
    fflush(stdout);
    bool ok = fourier_relay_transmit(&node_a, &node_b, TAG_KERNEL, &relay_gate);
    assert(ok == true);
    assert(relay_gate == CONDUC_STATE);
    assert(node_b.cosine_bin == 12.50);
    assert(node_b.sine_bin == -4.50);
    assert(node_a.seq == 11);
    assert(node_b.seq == 11);
    assert(memcmp(initial_hash_b, node_b.dat->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Transmission successful. Node B registry updated and sequence advanced.\n");
    fflush(stdout);

    // Save hash state of Node B
    uint8_t step1_hash_b[HASH_SIZE];
    memcpy(step1_hash_b, node_b.dat->node_hash, HASH_SIZE);

    // 2. Replay Attack Block (Node A attempts to re-transmit with stale sequence 10)
    printf("[TEST] Attempting replayed transmission from Node A (stale seq)...\n");
    fflush(stdout);
    node_a.seq = 10; // Forced rollback to simulate attacker replay

    ok = fourier_relay_transmit(&node_a, &node_b, TAG_KERNEL, &relay_gate);
    assert(ok == false);
    assert(relay_gate == CUTOFF_STATE); // Cutoff active
    assert(node_b.cosine_bin == 12.50); // Unaltered
    assert(memcmp(step1_hash_b, node_b.dat->node_hash, HASH_SIZE) == 0); // Hash unchanged
    printf("   ✓ Replay attempt blocked successfully by sequence check.\n");
    fflush(stdout);

    free(dat_a);
    free(dat_b);
    printf("=============================================================\n");
    printf("FOURIER NODE RELAY NETWORK TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
