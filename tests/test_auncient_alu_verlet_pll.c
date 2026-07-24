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

// Per-ALU Verlet, PLL, and XPLSM unified structure
typedef struct {
    uint32_t pll_phase;       // 0 to 359 degrees
    double charge_v;          // Verlet capacitor charge (V)
    double prev_charge_v;     // Verlet previous charge (V)
    double accel;             // Acceleration (V/s^2)
    TwoThreeNode *xplsm_dat;  // BASED registry leaf
    gate_state_t write_gate;
} alu_verlet_pll_t;

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
// Per-ALU Verlet + PLL + XPLSM Step
// -------------------------------------------------------------
bool alu_verlet_pll_step(alu_verlet_pll_t *alu, uint32_t phase_angle, 
                         double dt, double k_decay, uint8_t tag) {
    if (tag != TAG_KERNEL) return false;

    alu->pll_phase = phase_angle % 360;
    bool pll_locked = (alu->pll_phase == 0);

    // Verlet integration runs ONLY when PLL is phase-locked
    if (pll_locked) {
        alu->accel = -k_decay * alu->charge_v;
        double current_v = alu->charge_v;
        double next_v = (2.0 * current_v) - alu->prev_charge_v + (alu->accel * dt * dt);
        if (next_v < 0.0) next_v = 0.0;

        alu->prev_charge_v = current_v;
        alu->charge_v = next_v;

        // Save new charge state value directly to XPLSM registry
        char reg_val[128];
        snprintf(reg_val, sizeof(reg_val), "VOLTAGE:%.2f", alu->charge_v);
        strcpy(alu->xplsm_dat->values[0], reg_val);

        uint8_t temp[256];
        int len = snprintf((char*)temp, sizeof(temp), "%u:%s", alu->xplsm_dat->keys[0], alu->xplsm_dat->values[0]);
        sha256(temp, len, alu->xplsm_dat->node_hash);

        // NPN Conduction gate matches charge threshold
        alu->write_gate = (alu->charge_v >= 0.7) ? CONDUC_STATE : CUTOFF_STATE;
    }

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PER-ALU VERLET + PLL + XPLSM VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *dat = create_leaf(200, "VOLTAGE:5.00");
    alu_verlet_pll_t alu = {
        .pll_phase = 0,
        .charge_v = 5.0,
        .prev_charge_v = 5.0,
        .accel = 0.0,
        .xplsm_dat = dat,
        .write_gate = CONDUC_STATE
    };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, alu.xplsm_dat->node_hash, HASH_SIZE);

    // 1. Run step out-of-phase (180 deg) -> Verlet should not execute, hash remains unchanged
    printf("[TEST] Running step out-of-phase (180 deg)...\n");
    fflush(stdout);
    bool ok = alu_verlet_pll_step(&alu, 180, 0.1, 5.0, TAG_KERNEL);
    assert(ok == true);
    assert(alu.pll_phase == 180);
    assert(alu.charge_v == 5.0); // Unaltered
    assert(memcmp(initial_hash, alu.xplsm_dat->node_hash, HASH_SIZE) == 0); // Hash unchanged
    printf("   ✓ Out-of-phase cycle ignored Verlet integration step.\n");
    fflush(stdout);

    // 2. Run step in-phase (360 deg -> 0 deg) -> Verlet executes, updates registry
    printf("[TEST] Running step in-phase (360 deg)...\n");
    fflush(stdout);
    ok = alu_verlet_pll_step(&alu, 360, 0.1, 5.0, TAG_KERNEL);
    assert(ok == true);
    assert(alu.pll_phase == 0);
    assert(alu.charge_v == 4.75); // Decayed
    assert(memcmp(initial_hash, alu.xplsm_dat->node_hash, HASH_SIZE) != 0); // Hash changed
    printf("   ✓ Verlet step completed during phase lock. Registry updated.\n");
    fflush(stdout);

    free(dat);
    printf("=============================================================\n");
    printf("PER-ALU VERLET + PLL + XPLSM TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
