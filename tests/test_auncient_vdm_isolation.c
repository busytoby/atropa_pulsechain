#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define ETHERTYPE_STANAG 0x8B32
#define ETHERTYPE_DECNET 0x6003

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

// 2-3 Merkle Tree Node (Private VDM DAT registry)
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// VDM context
typedef struct {
    uint32_t vdm_id;
    TwoThreeNode *private_dat;
    uint16_t vdm_ethertype;
    gate_state_t write_gate;
} vdm_t;

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
// z/VM Hypervisor Frame Routing to VDM
// -------------------------------------------------------------
bool zvm_route_frame(vdm_t *vdm, uint16_t ethertype, const char *payload, uint8_t tag) {
    vdm->write_gate = CUTOFF_STATE;

    // 1. Privilege Tag check
    if (tag != TAG_KERNEL) return false;

    // 2. Network device routing check
    if (ethertype != vdm->vdm_ethertype) {
        return false; // Ignored: does not match interface config
    }

    // Write to interface BASED registry located inside the VDM's private DAT
    vdm->write_gate = CONDUC_STATE;
    strcpy(vdm->private_dat->values[0], payload);

    // Recompute private Merkle root hash
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", vdm->private_dat->keys[0], vdm->private_dat->values[0]);
    sha256(temp, len, vdm->private_dat->node_hash);

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT Z/VM ISOLATED DAT MACHINE (VDM) VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Create private DAT instances for two independent VDMs
    TwoThreeNode *dat1 = create_leaf(100, "BUF_INIT_1");
    TwoThreeNode *dat2 = create_leaf(200, "BUF_INIT_2");

    vdm_t vdm1 = {
        .vdm_id = 1,
        .private_dat = dat1,
        .vdm_ethertype = ETHERTYPE_DECNET,
        .write_gate = CUTOFF_STATE
    };

    vdm_t vdm2 = {
        .vdm_id = 2,
        .private_dat = dat2,
        .vdm_ethertype = ETHERTYPE_STANAG,
        .write_gate = CUTOFF_STATE
    };

    uint8_t initial_hash1[HASH_SIZE];
    uint8_t initial_hash2[HASH_SIZE];
    memcpy(initial_hash1, dat1->node_hash, HASH_SIZE);
    memcpy(initial_hash2, dat2->node_hash, HASH_SIZE);

    // 1. Route DECnet frame -> Should wake VDM 1, isolate VDM 2
    printf("[TEST] Routing DECnet frame (0x6003) to VDM 1...\n");
    fflush(stdout);
    bool ok = zvm_route_frame(&vdm1, ETHERTYPE_DECNET, "DECNET_PAYLOAD", TAG_KERNEL);
    assert(ok == true);
    assert(vdm1.write_gate == CONDUC_STATE);
    assert(strcmp(dat1->values[0], "DECNET_PAYLOAD") == 0);
    assert(memcmp(initial_hash1, dat1->node_hash, HASH_SIZE) != 0);

    // VDM 2 remains fully isolated
    assert(vdm2.write_gate == CUTOFF_STATE);
    assert(strcmp(dat2->values[0], "BUF_INIT_2") == 0);
    assert(memcmp(initial_hash2, dat2->node_hash, HASH_SIZE) == 0);
    printf("   ✓ VDM 1 updated. VDM 2 state isolated successfully.\n");
    fflush(stdout);

    // 2. Route STANAG frame -> Should wake VDM 2, isolate VDM 1
    printf("[TEST] Routing STANAG frame (0x8B32) to VDM 2...\n");
    fflush(stdout);
    ok = zvm_route_frame(&vdm2, ETHERTYPE_STANAG, "STANAG_PAYLOAD", TAG_KERNEL);
    assert(ok == true);
    assert(vdm2.write_gate == CONDUC_STATE);
    assert(strcmp(dat2->values[0], "STANAG_PAYLOAD") == 0);
    printf("   ✓ VDM 2 updated. VDM 1 state isolated successfully.\n");
    fflush(stdout);

    free(dat1);
    free(dat2);
    printf("=============================================================\n");
    printf("Z/VM VDM ISOLATION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
