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

// PL/I Attributed Variable
typedef struct {
    char name[16];
    char value[64];
    size_t length_limit;
    bool has_merkle;
    bool has_pll;
    bool has_audited;
    TwoThreeNode *merkle_node;
    uint32_t pll_phase;
    gate_state_t write_gate;
} pli_attributed_var_t;

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
// PL/I Attributed Assignment Engine
// -------------------------------------------------------------
bool pli_assign_attributed(pli_attributed_var_t *var, const char *new_val, 
                           uint32_t current_phase, uint8_t tag) {
    var->write_gate = CUTOFF_STATE;

    if (tag != TAG_KERNEL) return false;

    // 1. AUDITED Attribute Check (Implicit bounds checking)
    if (var->has_audited) {
        if (strlen(new_val) >= var->length_limit) {
            return false; // Bounds violation caught
        }
    }

    // 2. PLL Attribute Check (Implicit phase-lock gating)
    if (var->has_pll) {
        var->pll_phase = current_phase % 360;
        if (var->pll_phase != 0) {
            return false; // Delayed: waiting for clock phase lock
        }
    }

    // Conduction established
    var->write_gate = CONDUC_STATE;
    strcpy(var->value, new_val);

    // 3. MERKLE Attribute Check (Implicit Merkle path update)
    if (var->has_merkle && var->merkle_node) {
        strcpy(var->merkle_node->values[0], new_val);

        uint8_t temp[256];
        int len = snprintf((char*)temp, sizeof(temp), "%u:%s", var->merkle_node->keys[0], var->merkle_node->values[0]);
        sha256(temp, len, var->merkle_node->node_hash);
    }

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PL/I ATTRIBUTED VARIABLE VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *node = create_leaf(500, "INIT");
    pli_attributed_var_t var = {
        .name = "SysVar",
        .value = "INIT",
        .length_limit = 10,
        .has_merkle = true,
        .has_pll = true,
        .has_audited = true,
        .merkle_node = node,
        .pll_phase = 0,
        .write_gate = CUTOFF_STATE
    };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, node->node_hash, HASH_SIZE);

    // 1. Valid Assignment (In-phase, within limits) -> Should succeed and update Merkle hash
    printf("[TEST] Assigning valid value in-phase (360 deg)...\n");
    fflush(stdout);
    bool ok = pli_assign_attributed(&var, "VALID", 360, TAG_KERNEL);
    assert(ok == true);
    assert(var.write_gate == CONDUC_STATE);
    assert(strcmp(var.value, "VALID") == 0);
    assert(strcmp(node->values[0], "VALID") == 0);
    assert(memcmp(initial_hash, node->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Assignment completed. Merkle hash evolved successfully.\n");
    fflush(stdout);

    // Save current hash state
    uint8_t step1_hash[HASH_SIZE];
    memcpy(step1_hash, node->node_hash, HASH_SIZE);

    // 2. Block Assignment out-of-phase (180 deg) -> Should delay/cutoff
    printf("[TEST] Assigning value out-of-phase (180 deg)...\n");
    fflush(stdout);
    ok = pli_assign_attributed(&var, "DELAYED", 180, TAG_KERNEL);
    assert(ok == false);
    assert(var.write_gate == CUTOFF_STATE); // Cutoff active
    assert(strcmp(var.value, "VALID") == 0); // Unaltered
    assert(memcmp(step1_hash, node->node_hash, HASH_SIZE) == 0); // Hash unchanged
    printf("   ✓ Assignment delayed successfully by PLL clock phase gate.\n");
    fflush(stdout);

    // 3. Block Assignment due to AUDITED bounds check (Length 15 >= Limit 10) -> Should cutoff
    printf("[TEST] Assigning invalid length value (overflow)...\n");
    fflush(stdout);
    ok = pli_assign_attributed(&var, "OVERFLOW_PAYLOAD", 360, TAG_KERNEL);
    assert(ok == false);
    assert(var.write_gate == CUTOFF_STATE);
    printf("   ✓ Assignment blocked successfully by AUDITED bounds check.\n");
    fflush(stdout);

    free(node);
    printf("=============================================================\n");
    printf("PL/I ATTRIBUTED VARIABLE TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
