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

// PDL Shadow Recovery Entry
typedef struct {
    char before_value[128];
    uint8_t before_hash[HASH_SIZE];
} pdl_recovery_t;

// PDL Context
typedef struct {
    pdl_recovery_t recovery_log;
    gate_state_t write_gate;
} pdl_contract_ctx_t;

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
void pdl_rollback_contract(pdl_contract_ctx_t *ctx, TwoThreeNode *node) {
    strcpy(node->values[0], ctx->recovery_log.before_value);
    memcpy(node->node_hash, ctx->recovery_log.before_hash, HASH_SIZE);
    ctx->write_gate = CUTOFF_STATE;
}

// -------------------------------------------------------------
// PDL Contract Execution (alpha ; ?psi)
// -------------------------------------------------------------
bool pdl_execute_contract(pdl_contract_ctx_t *ctx, TwoThreeNode *node, 
                           const char *new_val, bool (*postcondition)(const TwoThreeNode *), 
                           uint8_t tag) {
    ctx->write_gate = CUTOFF_STATE;

    if (tag != TAG_KERNEL) return false;

    // 1. Shadow Log before-image
    strcpy(ctx->recovery_log.before_value, node->values[0]);
    memcpy(ctx->recovery_log.before_hash, node->node_hash, HASH_SIZE);

    // 2. Execute program alpha (assignment)
    ctx->write_gate = CONDUC_STATE;
    strcpy(node->values[0], new_val);

    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
    sha256(temp, len, node->node_hash);

    // 3. Evaluate postcondition ?psi
    if (!postcondition(node)) {
        // Postcondition violated: execute converse program alpha^-
        pdl_rollback_contract(ctx, node);
        return false; // Transaction aborted and rolled back
    }

    return true; // Transaction committed
}

// Postcondition contract check: registry string value must not contain spaces
bool postcondition_no_spaces(const TwoThreeNode *node) {
    return strchr(node->values[0], ' ') == NULL;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PDL CONTRACT GATING VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *node = create_leaf(500, "VAL_INITIAL");
    pdl_contract_ctx_t ctx = {
        .recovery_log = { .before_value = "" },
        .write_gate = CUTOFF_STATE
    };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, node->node_hash, HASH_SIZE);

    // 1. Compliant transaction (no spaces) -> Should commit successfully
    printf("[TEST] Executing compliant contract transaction (writing 'COMPLIANT_VAL')...\n");
    fflush(stdout);
    bool ok = pdl_execute_contract(&ctx, node, "COMPLIANT_VAL", postcondition_no_spaces, TAG_KERNEL);
    assert(ok == true);
    assert(ctx.write_gate == CONDUC_STATE);
    assert(strcmp(node->values[0], "COMPLIANT_VAL") == 0);
    assert(memcmp(initial_hash, node->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Transaction committed. Value = %s\n", node->values[0]);
    fflush(stdout);

    // Save hash state
    uint8_t commit_hash[HASH_SIZE];
    memcpy(commit_hash, node->node_hash, HASH_SIZE);

    // 2. Non-compliant transaction (contains spaces) -> Should trigger converse rollback
    printf("[TEST] Executing non-compliant transaction (writing 'UNSAFE VAL OUT')...\n");
    fflush(stdout);
    ok = pdl_execute_contract(&ctx, node, "UNSAFE VAL OUT", postcondition_no_spaces, TAG_KERNEL);
    assert(ok == false);
    assert(ctx.write_gate == CUTOFF_STATE); // Cutoff active
    // Value and hash rolled back to VAL_COMMIT_A
    assert(strcmp(node->values[0], "COMPLIANT_VAL") == 0);
    assert(memcmp(commit_hash, node->node_hash, HASH_SIZE) == 0);
    printf("   ✓ Postcondition check failed: rollback executed successfully.\n");
    fflush(stdout);

    free(node);
    printf("=============================================================\n");
    printf("PDL CONTRACT TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
