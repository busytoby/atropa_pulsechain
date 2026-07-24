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

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Interlocked Leaf Node
typedef struct {
    uint32_t key;
    char value[64];
    uint32_t nonce;
    uint32_t seq;
    uint8_t node_hash[HASH_SIZE];
} interlocked_leaf_t;

// -------------------------------------------------------------
// Interlocked Tamper verification Operations
// -------------------------------------------------------------

// Calculate sequence and nonce interlocked hash
void calculate_interlocked_hash(interlocked_leaf_t *leaf) {
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s:%u:%u", 
                       leaf->key, leaf->value, leaf->nonce, leaf->seq);
    sha256(temp, len, leaf->node_hash);
}

// Verify transaction challenge response and sequence (Replay Prevention)
bool verify_and_gate_transaction(interlocked_leaf_t *leaf, uint32_t expected_seq, 
                                 uint32_t response_nonce, const char *new_val, 
                                 gate_state_t *hardware_gate) {
    // 1. Verify sequence order (Anti-Replay check)
    if (leaf->seq != expected_seq) {
        *hardware_gate = CUTOFF_STATE;
        return false;
    }

    // 2. Challenge-Response verification: response_nonce must match current challenge nonce + 1
    if (response_nonce != leaf->nonce + 1) {
        *hardware_gate = CUTOFF_STATE;
        return false;
    }

    // 3. Both gates passed -> open conduction, update state, and advance sequence/nonce
    *hardware_gate = CONDUC_STATE;
    strcpy(leaf->value, new_val);
    leaf->nonce = response_nonce * 3; // Generate next challenge
    leaf->seq++;
    calculate_interlocked_hash(leaf);
    
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT INTERLOCKED TAMPER VERIFICATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initial state
    interlocked_leaf_t leaf = {
        .key = 100,
        .value = "DMA_PAYLOAD_0",
        .nonce = 1234, // Initial challenge nonce
        .seq = 10      // Initial expected sequence number
    };
    calculate_interlocked_hash(&leaf);

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, leaf.node_hash, HASH_SIZE);
    gate_state_t hw_gate = CUTOFF_STATE;

    // 1. Test Valid Transaction (Correct challenge response and sequence)
    printf("[TEST] Dispatching transaction with correct sequence and challenge response...\n");
    fflush(stdout);
    bool ok = verify_and_gate_transaction(&leaf, 10, 1235, "DMA_PAYLOAD_1", &hw_gate);
    assert(ok == true);
    assert(hw_gate == CONDUC_STATE);
    assert(strcmp(leaf.value, "DMA_PAYLOAD_1") == 0);
    assert(leaf.seq == 11);
    assert(memcmp(initial_hash, leaf.node_hash, HASH_SIZE) != 0);
    printf("   ✓ Transaction approved and executed successfully.\n");
    fflush(stdout);

    // Save state hash
    uint8_t step1_hash[HASH_SIZE];
    memcpy(step1_hash, leaf.node_hash, HASH_SIZE);

    // 2. Test Replay Attack (Resubmitting old sequence 10)
    printf("[TEST] Dispatching transaction with replayed sequence (10)...\n");
    fflush(stdout);
    ok = verify_and_gate_transaction(&leaf, 10, 1235, "REPLAY_TAMPER_DATA", &hw_gate);
    assert(ok == false);
    assert(hw_gate == CUTOFF_STATE); // Gated/Cutoff
    assert(strcmp(leaf.value, "DMA_PAYLOAD_1") == 0); // Unaltered
    assert(memcmp(step1_hash, leaf.node_hash, HASH_SIZE) == 0); // Hash unchanged
    printf("   ✓ Replay attempt blocked successfully by sequence interlock.\n");
    fflush(stdout);

    // 3. Test Invalid Challenge Response (Incorrect nonce)
    printf("[TEST] Dispatching transaction with invalid challenge response...\n");
    fflush(stdout);
    ok = verify_and_gate_transaction(&leaf, 11, 9999, "INVALID_CHALLENGE_DATA", &hw_gate);
    assert(ok == false);
    assert(hw_gate == CUTOFF_STATE); // Gated/Cutoff
    assert(strcmp(leaf.value, "DMA_PAYLOAD_1") == 0); // Unaltered
    printf("   ✓ Tamper attempt blocked successfully by challenge mismatch.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("INTERLOCKED TAMPER VERIFICATION TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
