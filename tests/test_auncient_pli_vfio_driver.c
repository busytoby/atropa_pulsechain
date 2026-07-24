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

// 2-3 Merkle Tree Node for BASED DMA Descriptors
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// PL/I-VFIO Driver Context
typedef struct {
    bool event_asserted;
    TwoThreeNode *dma_ring;
    uint8_t trans_table[256];
    gate_state_t driver_gate;
    char processed_payload[64];
} pli_vfio_driver_t;

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
// Unified PL/I VFIO Driver Processing Loop
// -------------------------------------------------------------
bool pli_vfio_process_interrupt(pli_vfio_driver_t *drv, uint32_t desc_key, 
                                 uint8_t tag) {
    drv->event_asserted = false;
    drv->driver_gate = CUTOFF_STATE;

    // 1. Gated interrupt execution check (TAG_KERNEL = 0x01)
    if (tag != TAG_KERNEL) return false;

    // Trigger PL/I EVENT
    drv->event_asserted = true;
    drv->driver_gate = CONDUC_STATE;

    // 2. Dereference BASED DMA descriptor in 2-3 tree DAT
    TwoThreeNode *node = drv->dma_ring;
    if (!node || !node->is_leaf || node->keys[0] != desc_key) {
        drv->driver_gate = CUTOFF_STATE;
        return false;
    }

    // Verify Merkle integrity of the descriptor
    uint8_t current_verify[HASH_SIZE];
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
    sha256(temp, len, current_verify);

    if (memcmp(current_verify, node->node_hash, HASH_SIZE) != 0) {
        drv->driver_gate = CUTOFF_STATE;
        return false; // Tampering detected: block execution
    }

    // 3. Constant-time TRANSLATE lookup of payload data
    const char *src = node->values[0];
    size_t src_len = strlen(src);
    if (src_len >= sizeof(drv->processed_payload)) {
        drv->driver_gate = CUTOFF_STATE;
        return false; // Bounds check overflow protection
    }

    for (size_t i = 0; i < src_len; i++) {
        uint8_t c = (uint8_t)src[i];
        drv->processed_payload[i] = (char)drv->trans_table[c];
    }
    drv->processed_payload[src_len] = '\0';

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PL/I VFIO DRIVER VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Create BASED DMA descriptor: key 100 holds EBCDIC-style uppercase payload
    TwoThreeNode *dma_desc = create_leaf(100, "HELLO_VFIO_DMA");
    pli_vfio_driver_t drv = {
        .event_asserted = false,
        .dma_ring = dma_desc,
        .trans_table = {0},
        .driver_gate = CUTOFF_STATE,
        .processed_payload = ""
    };

    // Initialize translate table with identity mapping
    for (int i = 0; i < 256; i++) {
        drv.trans_table[i] = i;
    }
    // Set ROT13 for uppercase character mapping
    for (int i = 'A'; i <= 'Z'; i++) {
        drv.trans_table[i] = 'A' + ((i - 'A' + 13) % 26);
    }

    // 1. Process valid interrupt (TAG_KERNEL, valid Merkle descriptor) -> Should execute and translate
    printf("[TEST] Processing valid VFIO interrupt...\n");
    fflush(stdout);
    bool ok = pli_vfio_process_interrupt(&drv, 100, TAG_KERNEL);
    assert(ok == true);
    assert(drv.event_asserted == true);
    assert(drv.driver_gate == CONDUC_STATE);
    assert(strcmp(drv.processed_payload, "URYYB_ISVB_QZN") == 0); // Translated
    printf("   ✓ Interrupt completed. Translated payload: %s\n", drv.processed_payload);
    fflush(stdout);

    // 2. Process unauthorized interrupt (TAG_USER) -> Should block and cutoff
    printf("[TEST] Processing unauthorized VFIO interrupt...\n");
    fflush(stdout);
    ok = pli_vfio_process_interrupt(&drv, 100, TAG_USER);
    assert(ok == false);
    assert(drv.event_asserted == false);
    assert(drv.driver_gate == CUTOFF_STATE); // Gated/Cutoff active
    printf("   ✓ Interrupt blocked successfully by tag enforcement.\n");
    fflush(stdout);

    // 3. Process tampered DMA descriptor -> Should block and cutoff
    printf("[TEST] Processing tampered DMA descriptor interrupt...\n");
    fflush(stdout);
    strcpy(dma_desc->values[0], "DIRECT_TAMPERED_DATA"); // Manual overwrite bypassing gate

    ok = pli_vfio_process_interrupt(&drv, 100, TAG_KERNEL);
    assert(ok == false);
    assert(drv.driver_gate == CUTOFF_STATE); // Gated/Cutoff active
    printf("   ✓ Tampering detected: execution blocked at descriptor boundary.\n");
    fflush(stdout);

    free(dma_desc);
    printf("=============================================================\n");
    printf("PL/I VFIO DRIVER TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
