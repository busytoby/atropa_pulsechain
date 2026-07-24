#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define ETHERTYPE_DECNET 0x6003
#define ETHERTYPE_STANAG 0x8B32

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

// 2-3 Merkle Tree Node (Private DAT)
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// Guest VDM Partition Configuration (No Shared Memory)
typedef struct {
    uint32_t vdm_id;
    uint16_t registered_ethertype;
    TwoThreeNode *private_dat;
} coaxial_vdm_t;

// Shadow log for converse recovery
typedef struct {
    char before_value[128];
    uint8_t before_hash[HASH_SIZE];
} backup_log_t;

// Coaxial Bus Controller State
typedef struct {
    gate_state_t write_gate;
    backup_log_t backup;
} coaxial_bus_t;

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
// Coaxial Bus Transfer Gated by Fourier Implication
// -------------------------------------------------------------
bool coaxial_bus_transfer(coaxial_bus_t *bus, coaxial_vdm_t *src, coaxial_vdm_t *dest, 
                          const char *payload, uint16_t ethertype, double freq, 
                          double q_factor, uint8_t tag) {
    bus->write_gate = CUTOFF_STATE;

    // 1. Shadow Log destination partition state before transaction
    strcpy(bus->backup.before_value, dest->private_dat->values[0]);
    memcpy(bus->backup.before_hash, dest->private_dat->node_hash, HASH_SIZE);

    // 2. Precondition check: Fourier Frequency parameters
    bool phi_fourier = (freq == 440.0 && q_factor >= 0.5);
    if (!phi_fourier) {
        return false; // Precondition violation: halt transaction
    }

    // 3. Postcondition check: Privilege and EtherType routing alignment
    bool psi_clearance = (tag == TAG_KERNEL && ethertype == dest->registered_ethertype);
    if (!psi_clearance) {
        // Rollback dest partition using converse backup log
        strcpy(dest->private_dat->values[0], bus->backup.before_value);
        memcpy(dest->private_dat->node_hash, bus->backup.before_hash, HASH_SIZE);
        return false;
    }

    // Both checks pass: establish conduction and deliver payload
    bus->write_gate = CONDUC_STATE;
    strcpy(dest->private_dat->values[0], payload);

    // Recompute target partition Merkle hash
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", dest->private_dat->keys[0], dest->private_dat->values[0]);
    sha256(temp, len, dest->private_dat->node_hash);

    (void)src; // Source tracking for auditing logs
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ALL-COAXIAL ONLY VDM VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Instantiate two independent, isolated guest VDM partitions
    TwoThreeNode *dat1 = create_leaf(100, "INIT_1");
    TwoThreeNode *dat2 = create_leaf(200, "INIT_2");

    coaxial_vdm_t vdm1 = { .vdm_id = 1, .registered_ethertype = ETHERTYPE_DECNET, .private_dat = dat1 };
    coaxial_vdm_t vdm2 = { .vdm_id = 2, .registered_ethertype = ETHERTYPE_STANAG, .private_dat = dat2 };

    coaxial_bus_t bus = { .write_gate = CUTOFF_STATE };

    uint8_t initial_hash2[HASH_SIZE];
    memcpy(initial_hash2, dat2->node_hash, HASH_SIZE);

    // 1. Authorized coaxial transfer (vdm1 -> vdm2 via STANAG, valid frequency/tag) -> Should succeed
    printf("[TEST] Transferring payload from VDM 1 to VDM 2 via STANAG...\n");
    fflush(stdout);
    bool ok = coaxial_bus_transfer(&bus, &vdm1, &vdm2, "PAYLOAD_STANAG", ETHERTYPE_STANAG, 440.0, 0.8, TAG_KERNEL);
    assert(ok == true);
    assert(bus.write_gate == CONDUC_STATE);
    assert(strcmp(dat2->values[0], "PAYLOAD_STANAG") == 0);
    assert(memcmp(initial_hash2, dat2->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Coaxial transfer completed. Target partition updated.\n");
    fflush(stdout);

    // Save hash state
    uint8_t step1_hash2[HASH_SIZE];
    memcpy(step1_hash2, dat2->node_hash, HASH_SIZE);

    // 2. Reject transfer due to incorrect frequency -> Should block
    printf("[TEST] Transferring payload with incorrect frequency (420.0)...\n");
    fflush(stdout);
    ok = coaxial_bus_transfer(&bus, &vdm1, &vdm2, "PAYLOAD_DETUNED", ETHERTYPE_STANAG, 420.0, 0.8, TAG_KERNEL);
    assert(ok == false);
    assert(bus.write_gate == CUTOFF_STATE);
    assert(strcmp(dat2->values[0], "PAYLOAD_STANAG") == 0);
    assert(memcmp(step1_hash2, dat2->node_hash, HASH_SIZE) == 0);
    printf("   ✓ Precondition check failed. Transfer blocked successfully.\n");
    fflush(stdout);

    // 3. Reject transfer due to tag mismatch -> Should block and rollback
    printf("[TEST] Transferring payload with invalid user privilege tag...\n");
    fflush(stdout);
    ok = coaxial_bus_transfer(&bus, &vdm1, &vdm2, "PAYLOAD_USER", ETHERTYPE_STANAG, 440.0, 0.8, TAG_USER);
    assert(ok == false);
    assert(bus.write_gate == CUTOFF_STATE);
    assert(strcmp(dat2->values[0], "PAYLOAD_STANAG") == 0);
    assert(memcmp(step1_hash2, dat2->node_hash, HASH_SIZE) == 0);
    printf("   ✓ Postcondition check failed. Rollback executed successfully.\n");
    fflush(stdout);

    free(dat1);
    free(dat2);
    printf("=============================================================\n");
    printf("ALL-COAXIAL ONLY VDM TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
