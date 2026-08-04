#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define RING_SIZE 2

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

// Simulated VFIO DMA Descriptor
typedef struct {
    uint64_t addr;
    uint32_t len;
    uint32_t status;
} vfio_dma_desc_t;

// 2-3 Merkle Tree Node
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// XPLSM VFIO State
typedef struct {
    TwoThreeNode *root;
    gate_state_t write_gate;
    uint64_t dma_base_addr;
    uint64_t dma_limit_addr;
    bool cache_invalidated;
    char audit_log[256];
} xplsm_vfio_t;

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

// Helper to parse ADDR from payload
static uint64_t parse_addr_from_payload(const char *payload) {
    const char *p = strstr(payload, "ADDR=");
    if (p) {
        return strtoull(p + 5, NULL, 16);
    }
    return 0;
}

// -------------------------------------------------------------
// XPLSM VFIO DMA Operations
// -------------------------------------------------------------
bool xplsm_vfio_dma_write(xplsm_vfio_t *vfio, uint32_t desc_idx, const char *payload, uint8_t tag) {
    // 1. Memory Range Protection
    uint64_t target_addr = parse_addr_from_payload(payload);
    if (target_addr != 0 && (target_addr < vfio->dma_base_addr || target_addr > vfio->dma_limit_addr)) {
        snprintf(vfio->audit_log, sizeof(vfio->audit_log), "BLOCKED: Address 0x%llX out of bounds", (unsigned long long)target_addr);
        return false; // Out of bounds access blocked
    }

    // 2. Privilege level write gate check
    vfio->write_gate = (tag == TAG_KERNEL) ? CONDUC_STATE : CUTOFF_STATE;

    if (vfio->write_gate == CONDUC_STATE) {
        bool ok = store_table_data(vfio->root, desc_idx, payload);
        if (ok) {
            // Cache Coherency Audit: successfully writing invalidates matching cache descriptor
            vfio->cache_invalidated = true;
        }
        return ok;
    }

    // 3. Blocked Write Auditing: log blocked write attempt
    snprintf(vfio->audit_log, sizeof(vfio->audit_log), "BLOCKED: Unauthorized write by tag 0x%02X", tag);
    return false;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPLSM VFIO DMA AUDITOR VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize VFIO shared memory tree with descriptor 0
    TwoThreeNode *leaf = create_leaf(0, "DESC0:ADDR=0x1000,LEN=64,STATUS=FREE");
    xplsm_vfio_t vfio = {
        .root = leaf,
        .write_gate = CUTOFF_STATE,
        .dma_base_addr = 0x1000,
        .dma_limit_addr = 0x2000,
        .cache_invalidated = false,
        .audit_log = ""
    };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, vfio.root->node_hash, HASH_SIZE);

    // 1. Authorized VFIO write (TAG_KERNEL) -> Should update state, root hash, and invalidate cache
    printf("[TEST] Writing to VFIO descriptor with TAG_KERNEL...\n");
    fflush(stdout);
    bool write_ok = xplsm_vfio_dma_write(&vfio, 0, "DESC0:ADDR=0x1000,LEN=64,STATUS=READY", TAG_KERNEL);
    assert(write_ok == true);
    assert(strcmp(vfio.root->values[0], "DESC0:ADDR=0x1000,LEN=64,STATUS=READY") == 0);
    assert(memcmp(initial_hash, vfio.root->node_hash, HASH_SIZE) != 0);
    assert(vfio.cache_invalidated == true);
    printf("   ✓ Write completed, root hash updated, and cache coherency invalidated successfully.\n");
    fflush(stdout);

    // Save updated hash state
    uint8_t updated_hash[HASH_SIZE];
    memcpy(updated_hash, vfio.root->node_hash, HASH_SIZE);

    // 2. Unauthorized VFIO write (TAG_USER) -> Should be blocked, hash constant, and logged
    printf("[TEST] Writing to VFIO descriptor with TAG_USER...\n");
    fflush(stdout);
    write_ok = xplsm_vfio_dma_write(&vfio, 0, "TAMPERED_DESC_DATA", TAG_USER);
    assert(write_ok == false);
    assert(strcmp(vfio.root->values[0], "DESC0:ADDR=0x1000,LEN=64,STATUS=READY") == 0); // Unchanged
    assert(memcmp(updated_hash, vfio.root->node_hash, HASH_SIZE) == 0); // Unchanged
    assert(strlen(vfio.audit_log) > 0);
    assert(strstr(vfio.audit_log, "Unauthorized") != NULL);
    printf("   ✓ Write blocked and unauthorized write correctly logged.\n");
    fflush(stdout);

    // 3. Memory Bounds Violation -> Should be blocked and logged
    printf("[TEST] Writing to VFIO descriptor with Out of Bounds Address...\n");
    fflush(stdout);
    write_ok = xplsm_vfio_dma_write(&vfio, 0, "DESC0:ADDR=0x99000,LEN=64,STATUS=READY", TAG_KERNEL);
    assert(write_ok == false);
    assert(strstr(vfio.audit_log, "out of bounds") != NULL);
    printf("   ✓ Address out of bounds check trapped and logged successfully.\n");
    fflush(stdout);

    // 4. Simulated direct hardware tampering (IOMMU bypass simulation) -> Hash must mismatch
    printf("[TEST] Simulating direct hardware tampering audit...\n");
    fflush(stdout);
    strcpy(vfio.root->values[0], "HARDWARE_TAMPER_ATTEMPT");
    
    uint8_t verify_hash[HASH_SIZE];
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", vfio.root->keys[0], vfio.root->values[0]);
    sha256(temp, len, verify_hash);

    assert(memcmp(verify_hash, vfio.root->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Tampering detected: dynamic hash diverges from Merkle registry.\n");
    fflush(stdout);

    free(leaf);
    printf("=============================================================\n");
    printf("XPLSM VFIO DMA AUDITOR INTEGRATION TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
