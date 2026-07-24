#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define ROUTE_LIMIT 3

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

// 2-3 Merkle Tree Node (Routing Table Entry)
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// DECnet Routing Table entry
typedef struct {
    uint32_t dest_node_id;
    uint32_t target_vdm_id;
} decnet_route_t;

// Router Context
typedef struct {
    decnet_route_t routes[ROUTE_LIMIT];
    uint32_t route_count;
    TwoThreeNode *table_node;
    gate_state_t write_gate;
    char backup_value[128];
    uint8_t backup_hash[HASH_SIZE];
} decnet_router_t;

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
// Update DECnet Route with Horning Aliasing Check
// -------------------------------------------------------------
bool decnet_update_route(decnet_router_t *router, uint32_t dest_node_id, uint32_t target_vdm_id, uint8_t tag) {
    if (tag != TAG_KERNEL) return false;

    // Horning Aliasing Check: Prevent loopback routing anomalies
    if (dest_node_id == target_vdm_id) {
        return false; // Loopback loop blocked
    }

    if (router->route_count >= ROUTE_LIMIT) return false;

    // Commit route
    router->routes[router->route_count].dest_node_id = dest_node_id;
    router->routes[router->route_count].target_vdm_id = target_vdm_id;
    router->route_count++;

    // Update Merkle routing table string representation
    snprintf(router->table_node->values[0], sizeof(router->table_node->values[0]), "ROUTES:%u", router->route_count);
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", router->table_node->keys[0], router->table_node->values[0]);
    sha256(temp, len, router->table_node->node_hash);

    return true;
}

// -------------------------------------------------------------
// Coaxial DECnet Packet Router Gated by Fourier Implication
// -------------------------------------------------------------
bool decnet_route_packet(decnet_router_t *router, uint32_t dest_node_id, 
                         const char *payload, double freq, double q_factor, uint8_t tag) {
    router->write_gate = CUTOFF_STATE;

    if (tag != TAG_KERNEL) return false;

    // 1. Log routing table state (for converse rollback)
    strcpy(router->backup_value, router->table_node->values[0]);
    memcpy(router->backup_hash, router->table_node->node_hash, HASH_SIZE);

    // 2. Precondition check: Fourier Frequency parameters
    bool phi_fourier = (freq == 440.0 && q_factor >= 0.5);
    if (!phi_fourier) return false;

    // 3. Postcondition check: Lookup target VDM mapping
    int target_idx = -1;
    for (uint32_t i = 0; i < router->route_count; i++) {
        if (router->routes[i].dest_node_id == dest_node_id) {
            target_idx = (int)i;
            break;
        }
    }

    if (target_idx == -1) {
        // Route mismatch: rollback table and abort
        strcpy(router->table_node->values[0], router->backup_value);
        memcpy(router->table_node->node_hash, router->backup_hash, HASH_SIZE);
        return false;
    }

    // Pass: establish conduction
    router->write_gate = CONDUC_STATE;
    (void)payload; // Payload delivered to target registers (simulated here)
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COAXIAL DECNET PHASE IV ROUTING VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *table_node = create_leaf(800, "ROUTES:0");
    decnet_router_t router = {
        .route_count = 0,
        .table_node = table_node,
        .write_gate = CUTOFF_STATE
    };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, table_node->node_hash, HASH_SIZE);

    // 1. Add valid route (Node 1 -> VDM 2) -> Should succeed
    printf("[TEST] Adding valid DECnet route (Node 1 -> VDM 2)...\n");
    fflush(stdout);
    bool ok = decnet_update_route(&router, 1, 2, TAG_KERNEL);
    assert(ok == true);
    assert(strcmp(table_node->values[0], "ROUTES:1") == 0);
    assert(memcmp(initial_hash, table_node->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Route registered. Table Merkle hash updated.\n");
    fflush(stdout);

    // 2. Reject circular loop route (Node 3 -> VDM 3) -> Should block (Horning Check)
    printf("[TEST] Adding invalid loopback route (Node 3 -> VDM 3)...\n");
    fflush(stdout);
    ok = decnet_update_route(&router, 3, 3, TAG_KERNEL);
    assert(ok == false);
    printf("   ✓ Circular loop route blocked successfully by Horning auditor.\n");
    fflush(stdout);

    // Save hash state
    uint8_t commit_hash[HASH_SIZE];
    memcpy(commit_hash, table_node->node_hash, HASH_SIZE);

    // 3. Route packet to valid destination node -> Should pass implication gate
    printf("[TEST] Routing packet to Node 1 via coaxial carrier...\n");
    fflush(stdout);
    ok = decnet_route_packet(&router, 1, "DECNET_PAYLOAD", 440.0, 0.8, TAG_KERNEL);
    assert(ok == true);
    assert(router.write_gate == CONDUC_STATE);
    printf("   ✓ Packet routed successfully.\n");
    fflush(stdout);

    // 4. Route packet to unmapped destination node -> Should fail and cutoff
    printf("[TEST] Routing packet to unmapped Node 99...\n");
    fflush(stdout);
    ok = decnet_route_packet(&router, 99, "DECNET_PAYLOAD", 440.0, 0.8, TAG_KERNEL);
    assert(ok == false);
    assert(router.write_gate == CUTOFF_STATE);
    printf("   ✓ Unknown destination blocked and cut off successfully.\n");
    fflush(stdout);

    free(table_node);
    printf("=============================================================\n");
    printf("COAXIAL DECNET ROUTING TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
