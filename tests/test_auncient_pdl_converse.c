#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define LOG_LIMIT 5

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

// 2-3 Merkle Tree Node (Registry Record)
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// PDL Converse Shadow Log Entry
typedef struct {
    uint32_t key;
    char before_value[128];
    uint8_t before_hash[HASH_SIZE];
} shadow_log_t;

// PDL Transaction Context
typedef struct {
    shadow_log_t recovery_stack[LOG_LIMIT];
    uint32_t sp;
    gate_state_t write_gate;
} pdl_tx_context_t;

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
// PDL Converse Program (alpha^-) state rollback
// -------------------------------------------------------------
void pdl_execute_converse(pdl_tx_context_t *ctx, TwoThreeNode *node) {
    if (ctx->sp == 0) return;

    // Pop before-image state from recovery stack
    ctx->sp--;
    shadow_log_t *log = &ctx->recovery_stack[ctx->sp];

    // Restore node value and cryptographic signature
    strcpy(node->values[0], log->before_value);
    memcpy(node->node_hash, log->before_hash, HASH_SIZE);

    ctx->write_gate = CONDUC_STATE;
}

// -------------------------------------------------------------
// PDL Transaction Execution with Shadow Logging
// -------------------------------------------------------------
bool pdl_execute_transaction(pdl_tx_context_t *ctx, TwoThreeNode *node, 
                             const char *new_val, uint8_t tag) {
    ctx->write_gate = CUTOFF_STATE;

    if (tag != TAG_KERNEL) return false;

    // 1. Shadow Logging: Record before-image to stack
    if (ctx->sp >= LOG_LIMIT) return false;

    shadow_log_t *log = &ctx->recovery_stack[ctx->sp];
    log->key = node->keys[0];
    strcpy(log->before_value, node->values[0]);
    memcpy(log->before_hash, node->node_hash, HASH_SIZE);
    ctx->sp++;

    // 2. Simulate Program alpha execution: Write new value
    strcpy(node->values[0], new_val);

    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
    sha256(temp, len, node->node_hash);

    // Simulate failure condition: if payload contains "ERROR", trigger rollback (alpha^-)
    if (strstr(new_val, "ERROR") != NULL) {
        pdl_execute_converse(ctx, node); // Execute converse program
        return false; // Transaction aborted and rolled back
    }

    ctx->write_gate = CONDUC_STATE;
    return true; // Transaction committed successfully
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PDL CONVERSE PROGRAM VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *node = create_leaf(500, "VAL_INITIAL");
    pdl_tx_context_t ctx = {
        .sp = 0,
        .write_gate = CUTOFF_STATE
    };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, node->node_hash, HASH_SIZE);

    // 1. Execute valid transaction -> Should commit new state
    printf("[TEST] Executing valid transaction (writing VAL_COMMIT_A)...\n");
    fflush(stdout);
    bool ok = pdl_execute_transaction(&ctx, node, "VAL_COMMIT_A", TAG_KERNEL);
    assert(ok == true);
    assert(ctx.write_gate == CONDUC_STATE);
    assert(strcmp(node->values[0], "VAL_COMMIT_A") == 0);
    assert(memcmp(initial_hash, node->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Transaction committed. Value = %s\n", node->values[0]);
    fflush(stdout);

    // Save hash state
    uint8_t commit_hash[HASH_SIZE];
    memcpy(commit_hash, node->node_hash, HASH_SIZE);

    // 2. Execute failing transaction -> Should execute converse program (alpha^-) and rollback
    printf("[TEST] Executing failing transaction (writing VAL_ERROR_TEMP)...\n");
    fflush(stdout);
    ok = pdl_execute_transaction(&ctx, node, "VAL_ERROR_TEMP", TAG_KERNEL);
    assert(ok == false);
    // State must be rolled back to commit_hash / VAL_COMMIT_A
    assert(strcmp(node->values[0], "VAL_COMMIT_A") == 0);
    assert(memcmp(commit_hash, node->node_hash, HASH_SIZE) == 0);
    printf("   ✓ Transaction failed: Converse program executed. State rolled back successfully.\n");
    fflush(stdout);

    free(node);
    printf("=============================================================\n");
    printf("PDL CONVERSE PROGRAM TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
