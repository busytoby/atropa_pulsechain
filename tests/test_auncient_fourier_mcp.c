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

// Fourier Node state representation
typedef struct {
    uint32_t target_key;
    gate_state_t write_gate;
} fourier_node_state_t;

// MCP Supervisor Context
typedef struct {
    uint32_t mcp_id;
    gate_state_t mcp_gate;
} mcp_supervisor_t;

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
// MCP Fourier Registry Read Interface
// -------------------------------------------------------------
bool mcp_read_fourier_registry(mcp_supervisor_t *mcp, const TwoThreeNode *node, 
                               uint32_t key, uint8_t tag, char *dest_buf) {
    mcp->mcp_gate = CUTOFF_STATE;

    if (!node) return false;

    if (node->is_leaf) {
        if (node->keys[0] == key) {
            // Verify node hash integrity
            uint8_t current_verify[HASH_SIZE];
            uint8_t temp[256];
            int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
            sha256(temp, len, current_verify);

            if (memcmp(current_verify, node->node_hash, HASH_SIZE) != 0) {
                return false; // Tampered node
            }

            // Gated conduction opens if tag is KERNEL
            if (tag == TAG_KERNEL) {
                mcp->mcp_gate = CONDUC_STATE;
                strcpy(dest_buf, node->values[0]);
                return true;
            }
        }
        return false;
    }

    return mcp_read_fourier_registry(mcp, node->children[0], key, tag, dest_buf);
}

// -------------------------------------------------------------
// MCP Administrative Cutoff Override
// -------------------------------------------------------------
void mcp_override_cutoff(mcp_supervisor_t *mcp, fourier_node_state_t *node_state) {
    // Force target Fourier node to CUTOFF_STATE (isolate faulty hardware line)
    node_state->write_gate = CUTOFF_STATE;
    mcp->mcp_gate = CONDUC_STATE;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT FOURIER MCP INTERACTION VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *root = create_leaf(500, "COS:12.45,SIN:-3.20");
    fourier_node_state_t node_state = { .target_key = 500, .write_gate = CONDUC_STATE };
    mcp_supervisor_t mcp = { .mcp_id = 9, .mcp_gate = CUTOFF_STATE };

    char dest_buf[128] = "";

    // 1. Authorized MCP Read (TAG_KERNEL) -> Should succeed
    printf("[TEST] MCP reading Fourier Registry key 500 (TAG_KERNEL)...\n");
    fflush(stdout);
    bool ok = mcp_read_fourier_registry(&mcp, root, 500, TAG_KERNEL, dest_buf);
    assert(ok == true);
    assert(mcp.mcp_gate == CONDUC_STATE);
    assert(strcmp(dest_buf, "COS:12.45,SIN:-3.20") == 0);
    printf("   ✓ Read successful: %s\n", dest_buf);
    fflush(stdout);

    // 2. Unauthorized MCP Read (TAG_USER) -> Should block and cutoff
    printf("[TEST] MCP reading Fourier Registry key 500 (TAG_USER)...\n");
    fflush(stdout);
    strcpy(dest_buf, "");
    ok = mcp_read_fourier_registry(&mcp, root, 500, TAG_USER, dest_buf);
    assert(ok == false);
    assert(mcp.mcp_gate == CUTOFF_STATE); // Cutoff active
    printf("   ✓ Read blocked successfully by privilege tag.\n");
    fflush(stdout);

    // 3. Administrative Override Cutoff -> Should force Fourier node to CUTOFF_STATE
    printf("[TEST] Triggering MCP administrative cutoff override...\n");
    fflush(stdout);
    assert(node_state.write_gate == CONDUC_STATE);
    
    mcp_override_cutoff(&mcp, &node_state);
    assert(node_state.write_gate == CUTOFF_STATE); // Isolated
    printf("   ✓ Fourier node state forced to CUTOFF successfully.\n");
    fflush(stdout);

    free(root);
    printf("=============================================================\n");
    printf("FOURIER MCP INTERACTION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
