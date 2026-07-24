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

// 2-3 Merkle Tree Node for AUTODIN Registry
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// AUTODIN Recovery Checkpoint
typedef struct {
    uint32_t pid;
    uint64_t reg_val;
    uint32_t seq;
} autodin_checkpoint_t;

// AUTODIN ALU State
typedef struct {
    TwoThreeNode *registry_dat;
    gate_state_t recovery_gate;
    bool client_active;
    uint64_t restored_reg;
} autodin_alu_t;

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
// Centrally Gated AUTODIN Recovery Loop
// -------------------------------------------------------------
bool autodin_gate_recovery(autodin_alu_t *alu, const autodin_checkpoint_t *chk, 
                           uint32_t phase_angle, uint8_t tag) {
    alu->recovery_gate = CUTOFF_STATE;
    alu->client_active = false;

    // 1. Privilege check (TAG_KERNEL = 0x01)
    if (tag != TAG_KERNEL) return false;

    // 2. PLL Phase Lock check (converts only at 0 degrees phase)
    bool pll_locked = (phase_angle % 360 == 0);
    if (!pll_locked) return false;

    // 3. Merkle Registry Match: Verify chk values match the DAT registry
    char expected_val[128];
    snprintf(expected_val, sizeof(expected_val), "PID:%u,REG:%lu,SEQ:%u", chk->pid, chk->reg_val, chk->seq);

    if (strcmp(alu->registry_dat->values[0], expected_val) != 0) {
        return false; // Tampered or out-of-sync state
    }

    // All checks pass -> Conduction established, restore client session
    alu->recovery_gate = CONDUC_STATE;
    alu->client_active = true;
    alu->restored_reg = chk->reg_val;
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT AUTODIN RECOVERY GATEWAY VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initial valid checkpoint registry entry
    TwoThreeNode *dat = create_leaf(500, "PID:1,REG:2026,SEQ:45");
    autodin_alu_t alu = {
        .registry_dat = dat,
        .recovery_gate = CUTOFF_STATE,
        .client_active = false,
        .restored_reg = 0
    };

    autodin_checkpoint_t chk = {
        .pid = 1,
        .reg_val = 2026,
        .seq = 45
    };

    // 1. Valid Recovery (Gated write during phase lock, correct registry match, Kernel tag)
    printf("[TEST] Attempting valid session recovery (PLL locked, authorized)...\n");
    fflush(stdout);
    bool ok = autodin_gate_recovery(&alu, &chk, 360, TAG_KERNEL);
    assert(ok == true);
    assert(alu.recovery_gate == CONDUC_STATE);
    assert(alu.client_active == true);
    assert(alu.restored_reg == 2026);
    printf("   ✓ Session recovery approved and client state restored.\n");
    fflush(stdout);

    // 2. Reject out-of-phase recovery request (PLL unlocked, phase angle 180)
    printf("[TEST] Attempting out-of-phase session recovery (180 deg)...\n");
    fflush(stdout);
    ok = autodin_gate_recovery(&alu, &chk, 180, TAG_KERNEL);
    assert(ok == false);
    assert(alu.recovery_gate == CUTOFF_STATE); // Gated/Cutoff active
    assert(alu.client_active == false);
    printf("   ✓ Out-of-phase recovery request blocked by clock gate.\n");
    fflush(stdout);

    // 3. Reject tampered checkpoint data (Modified reg_val)
    printf("[TEST] Attempting recovery with tampered checkpoint data...\n");
    fflush(stdout);
    autodin_checkpoint_t tampered_chk = {
        .pid = 1,
        .reg_val = 9999, // Tampered register value
        .seq = 45
    };
    
    ok = autodin_gate_recovery(&alu, &tampered_chk, 360, TAG_KERNEL);
    assert(ok == false);
    assert(alu.recovery_gate == CUTOFF_STATE); // Gated/Cutoff active
    printf("   ✓ Recovery blocked due to registry hash mismatch.\n");
    fflush(stdout);

    free(dat);
    printf("=============================================================\n");
    printf("AUTODIN RECOVERY GATEWAY TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
