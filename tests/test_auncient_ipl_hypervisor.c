#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define IPL_BOOT_ENTRY 0x1000

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

// 2-3 Merkle Tree Node (I/PL Boot Control Block)
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// VDM Guest State
typedef struct {
    uint32_t vdm_id;
    uint32_t PC;
    gate_state_t write_gate;
} vdm_guest_t;

// z/VM Hypervisor I/PL Context
typedef struct {
    bool ipl_complete_event;
    gate_state_t loader_gate;
} zvm_hypervisor_t;

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
// z/VM Hypervisor Initial Program Load (I/PL)
// -------------------------------------------------------------
bool zvm_ipl_load(vdm_guest_t *guest, zvm_hypervisor_t *hv, 
                  const TwoThreeNode *ipl_node, uint8_t tag) {
    hv->ipl_complete_event = false;
    hv->loader_gate = CUTOFF_STATE;
    guest->write_gate = CUTOFF_STATE;
    guest->PC = 0;

    // 1. Privilege Tag Check
    if (tag != TAG_KERNEL) return false;

    // 2. Verify Merkle integrity of the I/PL block
    uint8_t current_verify[HASH_SIZE];
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", ipl_node->keys[0], ipl_node->values[0]);
    sha256(temp, len, current_verify);

    if (memcmp(current_verify, ipl_node->node_hash, HASH_SIZE) != 0) {
        return false; // Boot halted: I/PL configuration signature invalid
    }

    // 3. Conduction open: Load boot entry point and trigger completion event
    hv->loader_gate = CONDUC_STATE;
    guest->write_gate = CONDUC_STATE;
    guest->PC = IPL_BOOT_ENTRY;
    hv->ipl_complete_event = true;

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT Z/VM HYPERVISOR I/PL BOOT VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Create I/PL boot control block: specifies VDM 1 boot entry parameters
    TwoThreeNode *ipl_node = create_leaf(999, "ENTRY:0x1000,MEM:64MB");

    vdm_guest_t guest = {
        .vdm_id = 1,
        .PC = 0,
        .write_gate = CUTOFF_STATE
    };

    zvm_hypervisor_t hv = {
        .ipl_complete_event = false,
        .loader_gate = CUTOFF_STATE
    };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, ipl_node->node_hash, HASH_SIZE);

    // 1. Authorized I/PL Boot (TAG_KERNEL, valid Merkle signature) -> Should boot VDM successfully
    printf("[TEST] Executing I/PL boot load sequence (TAG_KERNEL)...\n");
    fflush(stdout);
    bool ok = zvm_ipl_load(&guest, &hv, ipl_node, TAG_KERNEL);
    assert(ok == true);
    assert(hv.loader_gate == CONDUC_STATE);
    assert(guest.write_gate == CONDUC_STATE);
    assert(guest.PC == IPL_BOOT_ENTRY);
    assert(hv.ipl_complete_event == true);
    printf("   ✓ Boot successful: Guest PC = 0x%04X, Conduction Active.\n", guest.PC);
    fflush(stdout);

    // 2. Unauthorized Boot (TAG_USER) -> Should block and cutoff
    printf("[TEST] Executing I/PL boot load sequence (TAG_USER)...\n");
    fflush(stdout);
    ok = zvm_ipl_load(&guest, &hv, ipl_node, TAG_USER);
    assert(ok == false);
    assert(hv.loader_gate == CUTOFF_STATE);
    assert(guest.write_gate == CUTOFF_STATE);
    assert(guest.PC == 0);
    printf("   ✓ Boot blocked successfully by supervisor privilege checks.\n");
    fflush(stdout);

    // 3. Tampering Check: Modify I/PL configuration directly and verify boot fails
    printf("[TEST] Attempting boot with tampered I/PL configuration...\n");
    fflush(stdout);
    strcpy(ipl_node->values[0], "ENTRY:0x9999,MEM:999MB"); // Manual overwrite bypassing gate

    ok = zvm_ipl_load(&guest, &hv, ipl_node, TAG_KERNEL);
    assert(ok == false);
    assert(hv.loader_gate == CUTOFF_STATE);
    assert(guest.write_gate == CUTOFF_STATE);
    printf("   ✓ Tampering detected: I/PL boot sequence halted.\n");
    fflush(stdout);

    free(ipl_node);
    printf("=============================================================\n");
    printf("Z/VM HYPERVISOR I/PL TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
