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

typedef enum {
    TRANS_PENDING,
    TRANS_COMMITTED,
    TRANS_ABORTED
} trans_status_t;

// 2-3 Merkle Tree Node (Transaction Work Area - TWA)
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// CICS Transaction Context
typedef struct {
    char trans_id[8];
    uint32_t pll_phase;
    TwoThreeNode *twa_node;
    trans_status_t status;
    bool event_asserted;
    gate_state_t write_gate;
} cics_trans_t;

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
// PLL-Gated CICS Transaction Execution Loop
// -------------------------------------------------------------
bool cics_execute_transaction(cics_trans_t *trans, uint32_t phase_angle, 
                              const char *new_payload, uint8_t tag) {
    trans->event_asserted = false;
    trans->write_gate = CUTOFF_STATE;

    if (tag != TAG_KERNEL) return false;

    trans->pll_phase = phase_angle % 360;
    bool pll_locked = (trans->pll_phase == 0);

    // CICS transaction commits ONLY when the local PLL is phase-locked
    if (pll_locked) {
        trans->event_asserted = true;
        trans->write_gate = CONDUC_STATE;

        // Update TWA registry values
        strcpy(trans->twa_node->values[0], new_payload);

        // Recompute Merkle root hash of the TWA node
        uint8_t temp[256];
        int len = snprintf((char*)temp, sizeof(temp), "%u:%s", trans->twa_node->keys[0], trans->twa_node->values[0]);
        sha256(temp, len, trans->twa_node->node_hash);

        trans->status = TRANS_COMMITTED;
        return true;
    }

    return false; // Delayed/Pending: waiting for phase lock
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT CICS PLL-GATED TRANSACTION VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *twa = create_leaf(750, "CICS_TWA_INIT_DATA");
    cics_trans_t trans = {
        .trans_id = "TRN1",
        .pll_phase = 0,
        .twa_node = twa,
        .status = TRANS_PENDING,
        .event_asserted = false,
        .write_gate = CUTOFF_STATE
    };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, twa->node_hash, HASH_SIZE);

    // 1. Execute out-of-phase (180 deg) -> Should not commit, status remains pending
    printf("[TEST] Executing CICS transaction out-of-phase (180 deg)...\n");
    fflush(stdout);
    bool ok = cics_execute_transaction(&trans, 180, "TWA_UPDATE_COMMIT", TAG_KERNEL);
    assert(ok == false);
    assert(trans.status == TRANS_PENDING);
    assert(trans.write_gate == CUTOFF_STATE);
    assert(strcmp(twa->values[0], "CICS_TWA_INIT_DATA") == 0);
    assert(memcmp(initial_hash, twa->node_hash, HASH_SIZE) == 0);
    printf("   ✓ Transaction delayed successfully. State remains pending.\n");
    fflush(stdout);

    // 2. Execute in-phase (360 deg -> 0 deg) -> Should commit and update TWA
    printf("[TEST] Executing CICS transaction in-phase (360 deg)...\n");
    fflush(stdout);
    ok = cics_execute_transaction(&trans, 360, "TWA_UPDATE_COMMIT", TAG_KERNEL);
    assert(ok == true);
    assert(trans.status == TRANS_COMMITTED);
    assert(trans.write_gate == CONDUC_STATE);
    assert(strcmp(twa->values[0], "TWA_UPDATE_COMMIT") == 0);
    assert(memcmp(initial_hash, twa->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Transaction committed successfully during PLL phase-lock.\n");
    fflush(stdout);

    free(twa);
    printf("=============================================================\n");
    printf("CICS PLL-GATED TRANSACTION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
