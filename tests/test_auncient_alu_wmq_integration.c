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

// Mapped ALU & WinchesterMQ Unified Integration Context
typedef struct {
    uint32_t pll_phase;        // 0 to 359 degrees
    double wmq_req_v;          // WinchesterMQ REQ line voltage (V)
    double wmq_ack_v;          // WinchesterMQ ACK line voltage (V)
    char wmq_data_reg[64];     // WinchesterMQ SCSI data register
    TwoThreeNode *alu_soft_reg;// Mapped ALU software register node
    bool interrupt_asserted;
    gate_state_t task_gate;
} alu_wmq_sys_t;

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
// Unified ALU & WinchesterMQ System Step
// -------------------------------------------------------------
bool alu_wmq_sys_step(alu_wmq_sys_t *sys, uint32_t phase_angle, uint8_t tag) {
    sys->interrupt_asserted = false;
    sys->task_gate = CUTOFF_STATE;

    if (tag != TAG_KERNEL) return false;

    sys->pll_phase = phase_angle % 360;
    bool pll_locked = (sys->pll_phase == 0);

    // Hardware sync runs ONLY when PLL is phase-locked
    if (pll_locked) {
        // Assert interrupt event if SCSI request handshake matches: REQ high (>0.7V) and ACK low (<0.2V)
        if (sys->wmq_req_v > 0.7 && sys->wmq_ack_v < 0.2) {
            sys->interrupt_asserted = true;
            sys->task_gate = CONDUC_STATE;

            // Copy physical SCSI data directly to mapped ALU software register inside 2-3 tree DAT
            strcpy(sys->alu_soft_reg->values[0], sys->wmq_data_reg);

            // Recompute Merkle root hash of the registry node
            uint8_t temp[256];
            int len = snprintf((char*)temp, sizeof(temp), "%u:%s", sys->alu_soft_reg->keys[0], sys->alu_soft_reg->values[0]);
            sha256(temp, len, sys->alu_soft_reg->node_hash);
        }
    }

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ALU AND WINCHESTERMQ INTEGRATION VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *soft_reg = create_leaf(900, "INIT_STATE");
    alu_wmq_sys_t sys = {
        .pll_phase = 0,
        .wmq_req_v = 5.0, // Request active
        .wmq_ack_v = 0.0, // Acknowledge active
        .wmq_data_reg = "SCSI_COMMAND_CODE_0xFA",
        .alu_soft_reg = soft_reg,
        .interrupt_asserted = false,
        .task_gate = CUTOFF_STATE
    };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, soft_reg->node_hash, HASH_SIZE);

    // 1. Run out-of-phase step (180 deg) -> Should not trigger interrupt or register map
    printf("[TEST] Running out-of-phase step (180 deg)...\n");
    fflush(stdout);
    bool ok = alu_wmq_sys_step(&sys, 180, TAG_KERNEL);
    assert(ok == true);
    assert(sys.interrupt_asserted == false);
    assert(sys.task_gate == CUTOFF_STATE);
    assert(strcmp(soft_reg->values[0], "INIT_STATE") == 0);
    assert(memcmp(initial_hash, soft_reg->node_hash, HASH_SIZE) == 0);
    printf("   ✓ Out-of-phase cycle ignored SCSI transaction successfully.\n");
    fflush(stdout);

    // 2. Run in-phase locked step (360 deg -> 0 deg) -> Should trigger interrupt and update register
    printf("[TEST] Running in-phase phase-locked step...\n");
    fflush(stdout);
    ok = alu_wmq_sys_step(&sys, 360, TAG_KERNEL);
    assert(ok == true);
    assert(sys.interrupt_asserted == true);
    assert(sys.task_gate == CONDUC_STATE);
    assert(strcmp(soft_reg->values[0], "SCSI_COMMAND_CODE_0xFA") == 0);
    assert(memcmp(initial_hash, soft_reg->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Phase-locked interrupt asserted. Mapped register updated successfully.\n");
    fflush(stdout);

    free(soft_reg);
    printf("=============================================================\n");
    printf("ALU AND WINCHESTERMQ INTEGRATION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
