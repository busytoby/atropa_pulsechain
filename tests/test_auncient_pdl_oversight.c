#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define NESTED_LIMIT 5

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

// VDM Partition Context
typedef struct {
    uint32_t vdm_id;
    TwoThreeNode *private_dat;
} oversight_vdm_t;

// Shadow Recovery Log Entry for Nested Transactions
typedef struct {
    oversight_vdm_t *vdm;
    char before_value[128];
    uint8_t before_hash[HASH_SIZE];
} nested_log_t;

// PDL Oversight Supervisor Context
typedef struct {
    nested_log_t recovery_stack[NESTED_LIMIT];
    uint32_t sp;
    gate_state_t write_gate;
} pdl_oversight_t;

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
// Shadow Log VDM State to Recovery Stack
// -------------------------------------------------------------
void pdl_oversight_log(pdl_oversight_t *supervisor, oversight_vdm_t *vdm) {
    if (supervisor->sp >= NESTED_LIMIT) return;

    nested_log_t *log = &supervisor->recovery_stack[supervisor->sp];
    log->vdm = vdm;
    strcpy(log->before_value, vdm->private_dat->values[0]);
    memcpy(log->before_hash, vdm->private_dat->node_hash, HASH_SIZE);
    supervisor->sp++;
}

// -------------------------------------------------------------
// Atomic Nested Converse Rollback
// -------------------------------------------------------------
void pdl_oversight_rollback(pdl_oversight_t *supervisor) {
    // Pop and restore all logged VDM states in reverse order
    while (supervisor->sp > 0) {
        supervisor->sp--;
        nested_log_t *log = &supervisor->recovery_stack[supervisor->sp];
        strcpy(log->vdm->private_dat->values[0], log->before_value);
        memcpy(log->vdm->private_dat->node_hash, log->before_hash, HASH_SIZE);
    }
    supervisor->write_gate = CUTOFF_STATE;
}

// -------------------------------------------------------------
// Nested Gated Write Transaction
// -------------------------------------------------------------
bool pdl_execute_nested_tx(pdl_oversight_t *supervisor, oversight_vdm_t *vdm1, 
                            oversight_vdm_t *vdm2, const char *val1, const char *val2, 
                            uint8_t tag) {
    supervisor->write_gate = CUTOFF_STATE;

    if (tag != TAG_KERNEL) return false;

    // 1. Log before-images for both VDM partitions
    pdl_oversight_log(supervisor, vdm1);
    pdl_oversight_log(supervisor, vdm2);

    // 2. Perform first write (vdm1)
    strcpy(vdm1->private_dat->values[0], val1);
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", vdm1->private_dat->keys[0], vdm1->private_dat->values[0]);
    sha256(temp, len, vdm1->private_dat->node_hash);

    // 3. Perform second write (vdm2)
    strcpy(vdm2->private_dat->values[0], val2);
    len = snprintf((char*)temp, sizeof(temp), "%u:%s", vdm2->private_dat->keys[0], vdm2->private_dat->values[0]);
    sha256(temp, len, vdm2->private_dat->node_hash);

    // Simulate nested execution failure check: if val2 has "ERROR", abort transaction
    if (strstr(val2, "ERROR") != NULL) {
        // Trigger atomic rollback of all nested modifications
        pdl_oversight_rollback(supervisor);
        return false;
    }

    supervisor->write_gate = CONDUC_STATE;
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PDL SUPERVISOR OVERSIGHT VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *dat1 = create_leaf(100, "INIT_1");
    TwoThreeNode *dat2 = create_leaf(200, "INIT_2");

    oversight_vdm_t vdm1 = { .vdm_id = 1, .private_dat = dat1 };
    oversight_vdm_t vdm2 = { .vdm_id = 2, .private_dat = dat2 };

    pdl_oversight_t supervisor = {
        .sp = 0,
        .write_gate = CUTOFF_STATE
    };

    uint8_t initial_hash1[HASH_SIZE];
    uint8_t initial_hash2[HASH_SIZE];
    memcpy(initial_hash1, dat1->node_hash, HASH_SIZE);
    memcpy(initial_hash2, dat2->node_hash, HASH_SIZE);

    // 1. Valid nested transaction -> Should commit both
    printf("[TEST] Executing valid nested transaction...\n");
    fflush(stdout);
    bool ok = pdl_execute_nested_tx(&supervisor, &vdm1, &vdm2, "COMMIT_A", "COMMIT_B", TAG_KERNEL);
    assert(ok == true);
    assert(supervisor.write_gate == CONDUC_STATE);
    assert(strcmp(dat1->values[0], "COMMIT_A") == 0);
    assert(strcmp(dat2->values[0], "COMMIT_B") == 0);
    assert(memcmp(initial_hash1, dat1->node_hash, HASH_SIZE) != 0);
    assert(memcmp(initial_hash2, dat2->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Nested transaction committed. VDM 1 = %s, VDM 2 = %s\n", dat1->values[0], dat2->values[0]);
    fflush(stdout);

    // Save hash states
    uint8_t commit_hash1[HASH_SIZE];
    uint8_t commit_hash2[HASH_SIZE];
    memcpy(commit_hash1, dat1->node_hash, HASH_SIZE);
    memcpy(commit_hash2, dat2->node_hash, HASH_SIZE);

    // Reset stack pointer
    supervisor.sp = 0;

    // 2. Failing nested transaction -> Should roll back both atomically
    printf("[TEST] Executing nested transaction with failure injection...\n");
    fflush(stdout);
    ok = pdl_execute_nested_tx(&supervisor, &vdm1, &vdm2, "NEW_VAL_1", "ERROR_VAL_2", TAG_KERNEL);
    assert(ok == false);
    assert(supervisor.write_gate == CUTOFF_STATE);
    // Both partitions must be rolled back to commit_hash1 / commit_hash2
    assert(strcmp(dat1->values[0], "COMMIT_A") == 0);
    assert(strcmp(dat2->values[0], "COMMIT_B") == 0);
    assert(memcmp(commit_hash1, dat1->node_hash, HASH_SIZE) == 0);
    assert(memcmp(commit_hash2, dat2->node_hash, HASH_SIZE) == 0);
    printf("   ✓ Transaction aborted. Atomic nested converse rollback succeeded.\n");
    fflush(stdout);

    free(dat1);
    free(dat2);
    printf("=============================================================\n");
    printf("PDL OVERSIGHT TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
