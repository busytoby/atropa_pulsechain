#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define MOTZKIN_PRIME 953467954114363ULL

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
static void sha256(const void *data, size_t len, uint8_t *out) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(out, &ctx);
}
#pragma GCC diagnostic pop

// Conduction states
typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Opcodes
typedef enum {
    OP_WINCHESTER,
    OP_NETWORK,
    OP_ABI_CHECK
} alu_op_t;

// 2-3 Merkle Tree Node
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// Unified ALU Structure
typedef struct {
    // WinchesterMQ SCSI Registers
    uint64_t base_reg;
    uint64_t signal_reg;
    uint64_t channel_reg;
    
    // Network DAT Registers
    TwoThreeNode *network_dat;
    
    // ABI Accessor Registers
    uint8_t active_tag;
    gate_state_t write_gate;
} unified_alu_t;

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

// Modular multiplication helper
static uint64_t mod_mult(uint64_t a, uint64_t b, uint64_t mod) {
    uint64_t res = 0;
    a %= mod;
    while (b > 0) {
        if (b & 1) res = (res + a) % mod;
        a = (2 * a) % mod;
        b >>= 1;
    }
    return res;
}

// Modular exponentiation helper
static uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) res = mod_mult(res, base, mod);
        base = mod_mult(base, base, mod);
        exp >>= 1;
    }
    return res;
}

// -------------------------------------------------------------
// Unified ALU Dispatch Execution Loop
// -------------------------------------------------------------
void unified_alu_dispatch(unified_alu_t *alu, alu_op_t op, uint64_t val, const char *str_val) {
    switch (op) {
        case OP_WINCHESTER:
            // Modular exponentiation: Channel = Base^Signal mod MotzkinPrime
            alu->channel_reg = mod_pow(alu->base_reg, alu->signal_reg, MOTZKIN_PRIME);
            break;
            
        case OP_NETWORK:
            // Write payload to Network DAT if write gate is conducting
            if (alu->write_gate == CONDUC_STATE) {
                store_table_data(alu->network_dat, (uint32_t)val, str_val);
            }
            break;
            
        case OP_ABI_CHECK:
            // Route active tag check to write gate (0x01 is authorized Kernel tag)
            alu->write_gate = (val == 0x01) ? CONDUC_STATE : CUTOFF_STATE;
            break;
    }
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT UNIFIED ALU DISPATCH VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *net_leaf = create_leaf(300, "STANAG_PKTS:0");
    unified_alu_t alu = {
        .base_reg = 2,
        .signal_reg = 10,
        .channel_reg = 0,
        .network_dat = net_leaf,
        .active_tag = 0,
        .write_gate = CUTOFF_STATE
    };

    // 1. Validate OP_WINCHESTER (SCSI math execution)
    printf("[TEST] Dispatching OP_WINCHESTER modular exponentiation...\n");
    fflush(stdout);
    unified_alu_dispatch(&alu, OP_WINCHESTER, 0, NULL);
    assert(alu.channel_reg == 1024); // 2^10 = 1024
    printf("   ✓ Winchester channel transition verified: %lu\n", alu.channel_reg);
    fflush(stdout);

    // 2. Validate OP_ABI_CHECK & OP_NETWORK (Denied with GUEST tag)
    printf("[TEST] Dispatching OP_ABI_CHECK with unauthorized tag (0x02)...\n");
    fflush(stdout);
    unified_alu_dispatch(&alu, OP_ABI_CHECK, 0x02, NULL);
    assert(alu.write_gate == CUTOFF_STATE);
    
    printf("[TEST] Dispatching OP_NETWORK (should be blocked by cutoff)...\n");
    fflush(stdout);
    unified_alu_dispatch(&alu, OP_NETWORK, 300, "STANAG_PKTS:10");
    assert(strcmp(alu.network_dat->values[0], "STANAG_PKTS:0") == 0); // Unchanged
    printf("   ✓ Network write blocked successfully by ABI gate.\n");
    fflush(stdout);

    // 3. Validate OP_ABI_CHECK & OP_NETWORK (Allowed with KERNEL tag)
    printf("[TEST] Dispatching OP_ABI_CHECK with authorized tag (0x01)...\n");
    fflush(stdout);
    unified_alu_dispatch(&alu, OP_ABI_CHECK, 0x01, NULL);
    assert(alu.write_gate == CONDUC_STATE);
    
    printf("[TEST] Dispatching OP_NETWORK (should succeed under conduction)...\n");
    fflush(stdout);
    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, alu.network_dat->node_hash, HASH_SIZE);

    unified_alu_dispatch(&alu, OP_NETWORK, 300, "STANAG_PKTS:10");
    assert(strcmp(alu.network_dat->values[0], "STANAG_PKTS:10") == 0); // Updated
    assert(memcmp(initial_hash, alu.network_dat->node_hash, HASH_SIZE) != 0); // Root hash evolved
    printf("   ✓ Network write executed and Merkle DAT evolved successfully.\n");
    fflush(stdout);

    free(net_leaf);
    printf("=============================================================\n");
    printf("UNIFIED ALU INTEGRATION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
