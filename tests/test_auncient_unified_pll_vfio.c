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

// 2-3 Merkle Tree Node
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// Unified PLL and XPLSM VFIO Structure
typedef struct {
    uint32_t pll_phase_angle; // 0 to 359 degrees
    bool pll_locked;
    TwoThreeNode *vfio_dat;
    gate_state_t write_gate;
} pll_vfio_alu_t;

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

// Update leaf node hash
static void update_node_hash(TwoThreeNode *node) {
    if (!node) return;
    if (node->is_leaf) {
        uint8_t temp[256];
        int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
        sha256(temp, len, node->node_hash);
    }
}

// Update leaf in 2-3 tree DAT
static bool store_table_data(TwoThreeNode *node, uint32_t key, const char *new_value) {
    if (!node) return false;
    if (node->is_leaf) {
        if (node->keys[0] == key) {
            strcpy(node->values[0], new_value);
            update_node_hash(node);
            return true;
        }
        return false;
    }
    return store_table_data(node->children[0], key, new_value);
}

// -------------------------------------------------------------
// Unified PLL + VFIO DMA Dispatch Loop
// -------------------------------------------------------------
bool dispatch_pll_vfio_write(pll_vfio_alu_t *alu, uint32_t desc_idx, 
                             const char *payload, uint32_t phase_angle, uint8_t tag) {
    alu->pll_phase_angle = phase_angle % 360;
    // PLL is locked only at zero-crossing points (0 degrees phase)
    alu->pll_locked = (alu->pll_phase_angle == 0);

    // Conduction occurs only if: Active tag is TAG_KERNEL AND PLL is locked
    alu->write_gate = (tag == TAG_KERNEL && alu->pll_locked) ? CONDUC_STATE : CUTOFF_STATE;

    if (alu->write_gate == CONDUC_STATE) {
        return store_table_data(alu->vfio_dat, desc_idx, payload);
    }
    return false; // Gated/Cutoff
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT UNIFIED PLL + VFIO XPLSM VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *vfio_leaf = create_leaf(50, "VFIO_DESC_INIT");
    pll_vfio_alu_t alu = {
        .pll_phase_angle = 180,
        .pll_locked = false,
        .vfio_dat = vfio_leaf,
        .write_gate = CUTOFF_STATE
    };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, alu.vfio_dat->node_hash, HASH_SIZE);

    // 1. Gated write during phase lock (0 degrees phase) & Kernel tag -> Should succeed
    printf("[TEST] Writing to VFIO descriptor during PLL phase lock (0 deg)...\n");
    fflush(stdout);
    bool ok = dispatch_pll_vfio_write(&alu, 50, "VFIO_DESC_LOCKED", 360, TAG_KERNEL);
    assert(ok == true);
    assert(alu.pll_locked == true);
    assert(alu.write_gate == CONDUC_STATE);
    assert(strcmp(alu.vfio_dat->values[0], "VFIO_DESC_LOCKED") == 0);
    assert(memcmp(initial_hash, alu.vfio_dat->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Write completed successfully. Merkle root evolved.\n");
    fflush(stdout);

    // Save updated hash
    uint8_t locked_hash[HASH_SIZE];
    memcpy(locked_hash, alu.vfio_dat->node_hash, HASH_SIZE);

    // 2. Gated write out-of-phase (90 degrees phase) -> Should be blocked/cutoff
    printf("[TEST] Writing to VFIO descriptor out-of-phase (90 deg)...\n");
    fflush(stdout);
    ok = dispatch_pll_vfio_write(&alu, 50, "VFIO_DESC_OUT_OF_PHASE", 90, TAG_KERNEL);
    assert(ok == false);
    assert(alu.pll_locked == false);
    assert(alu.write_gate == CUTOFF_STATE); // Gated/Cutoff active
    assert(strcmp(alu.vfio_dat->values[0], "VFIO_DESC_LOCKED") == 0); // Unchanged
    assert(memcmp(locked_hash, alu.vfio_dat->node_hash, HASH_SIZE) == 0); // Hash unchanged
    printf("   ✓ Out-of-phase write blocked successfully by PLL gate.\n");
    fflush(stdout);

    // 3. Gated write out-of-privilege (0 degrees phase but USER tag) -> Should be blocked
    printf("[TEST] Writing to VFIO descriptor with USER tag during phase lock...\n");
    fflush(stdout);
    ok = dispatch_pll_vfio_write(&alu, 50, "VFIO_DESC_USER_TAMPER", 0, TAG_USER);
    assert(ok == false);
    assert(alu.pll_locked == true);
    assert(alu.write_gate == CUTOFF_STATE); // Gated/Cutoff active
    assert(strcmp(alu.vfio_dat->values[0], "VFIO_DESC_LOCKED") == 0); // Unchanged
    printf("   ✓ Unauthorized write blocked successfully during active lock.\n");
    fflush(stdout);

    free(vfio_leaf);
    printf("=============================================================\n");
    printf("UNIFIED PLL + VFIO XPLSM INTEGRATION VERIFIED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
