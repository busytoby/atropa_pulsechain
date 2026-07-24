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

// 2-3 Merkle Tree Node
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// Shadow Recovery Entry
typedef struct {
    char before_value[128];
    uint8_t before_hash[HASH_SIZE];
} recovery_entry_t;

// VDM Context with Frequency Parameters
typedef struct {
    double freq;
    double q_factor;
    uint32_t address;
    gate_state_t write_gate;
    recovery_entry_t recovery_log;
} fourier_implication_ctx_t;

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
// PDL Converse Rollback (alpha^-)
// -------------------------------------------------------------
void pdl_rollback_fourier(fourier_implication_ctx_t *ctx, TwoThreeNode *node) {
    strcpy(node->values[0], ctx->recovery_log.before_value);
    memcpy(node->node_hash, ctx->recovery_log.before_hash, HASH_SIZE);
    ctx->write_gate = CUTOFF_STATE;
}

// -------------------------------------------------------------
// PDL Fourier Implication Gating: ?phi_fourier ; ?psi_clearance
// -------------------------------------------------------------
bool pdl_execute_fourier_implication(fourier_implication_ctx_t *ctx, TwoThreeNode *node, 
                                     const char *new_val, double target_freq, double min_q, 
                                     uint8_t tag) {
    ctx->write_gate = CUTOFF_STATE;

    // 1. Log before-image for converse recovery
    strcpy(ctx->recovery_log.before_value, node->values[0]);
    memcpy(ctx->recovery_log.before_hash, node->node_hash, HASH_SIZE);

    // 2. Precondition check ?phi_fourier (Frequency parameters)
    bool phi_fourier = (ctx->freq == target_freq && ctx->q_factor >= min_q);
    if (!phi_fourier) {
        return false; // Precondition failed: halt on entry
    }

    // 3. Postcondition check ?psi_clearance (Privilege & address limits)
    bool psi_clearance = (tag == TAG_KERNEL && ctx->address < 1000);
    if (!psi_clearance) {
        return false; // Postcondition failed: halt
    }

    // Establish conduction and commit write
    ctx->write_gate = CONDUC_STATE;
    strcpy(node->values[0], new_val);

    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
    sha256(temp, len, node->node_hash);

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT FOURIER-GATED IMPLICATION VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *node = create_leaf(500, "VAL_INITIAL");
    fourier_implication_ctx_t ctx = {
        .freq = 440.0,
        .q_factor = 0.8,
        .address = 500,
        .write_gate = CUTOFF_STATE
    };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, node->node_hash, HASH_SIZE);

    // 1. Both precondition (freq=440.0, q=0.8) and postcondition pass -> Should succeed
    printf("[TEST] Executing implication with valid frequency and clearance properties...\n");
    fflush(stdout);
    bool ok = pdl_execute_fourier_implication(&ctx, node, "VAL_COMMIT_A", 440.0, 0.5, TAG_KERNEL);
    assert(ok == true);
    assert(ctx.write_gate == CONDUC_STATE);
    assert(strcmp(node->values[0], "VAL_COMMIT_A") == 0);
    assert(memcmp(initial_hash, node->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Gated transaction committed successfully.\n");
    fflush(stdout);

    // Save hash state
    uint8_t commit_hash[HASH_SIZE];
    memcpy(commit_hash, node->node_hash, HASH_SIZE);

    // 2. Precondition fails (Frequency de-tuned) -> Should block and rollback
    printf("[TEST] Executing implication with de-tuned frequency (420.0)...\n");
    fflush(stdout);
    ctx.freq = 420.0; // de-tuned
    ok = pdl_execute_fourier_implication(&ctx, node, "VAL_BAD_WRITE", 440.0, 0.5, TAG_KERNEL);
    assert(ok == false);
    assert(ctx.write_gate == CUTOFF_STATE);
    assert(strcmp(node->values[0], "VAL_COMMIT_A") == 0);
    assert(memcmp(commit_hash, node->node_hash, HASH_SIZE) == 0);
    printf("   ✓ Frequency precondition violation blocked and rolled back successfully.\n");
    fflush(stdout);

    // Reset frequency
    ctx.freq = 440.0;

    // 3. Postcondition fails (Address out of bounds) -> Should block and rollback
    printf("[TEST] Executing implication with address out of bounds (9999)...\n");
    fflush(stdout);
    ctx.address = 9999; // out of bounds
    ok = pdl_execute_fourier_implication(&ctx, node, "VAL_BAD_WRITE", 440.0, 0.5, TAG_KERNEL);
    assert(ok == false);
    assert(ctx.write_gate == CUTOFF_STATE);
    assert(strcmp(node->values[0], "VAL_COMMIT_A") == 0);
    assert(memcmp(commit_hash, node->node_hash, HASH_SIZE) == 0);
    printf("   ✓ Clearance postcondition violation blocked and rolled back successfully.\n");
    fflush(stdout);

    free(node);
    printf("=============================================================\n");
    printf("FOURIER-GATED IMPLICATION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
