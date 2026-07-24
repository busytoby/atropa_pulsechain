#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define Q_MIN_LIMIT 0.5
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

// 2-3 Merkle Tree Node
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// Fourier Protocol Task Context
typedef struct {
    uint32_t device_id;
    uint16_t ethertype;
    bool event_asserted;
    struct {
        uint32_t target_key;
        uint32_t seq;
    } ptr;
    TwoThreeNode *dat;
    gate_state_t write_gate;
} fourier_task_t;

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
// Fourier Protocol VFIO Driver Processing Loop
// -------------------------------------------------------------
bool fourier_protocol_process(fourier_task_t *task, uint16_t ethertype, 
                              double cosine_bin, double sine_bin, double noise_floor, 
                              uint8_t tag) {
    task->event_asserted = false;
    task->write_gate = CUTOFF_STATE;

    // 1. EtherType Demux Check
    if (ethertype != task->ethertype) {
        return false; // Wrong interface routing
    }

    // 2. Q-Factor Bounding check
    if (noise_floor <= 0.0) return false;
    double amplitude = sqrt((cosine_bin * cosine_bin) + (sine_bin * sine_bin));
    double q_factor = amplitude / noise_floor;
    if (q_factor < Q_MIN_LIMIT) {
        return false; // Decoupled carrier signal cutoff
    }

    // 3. Privilege Tag check
    if (tag != TAG_KERNEL) {
        return false;
    }

    // Assert PL/I Event and open write-gate
    task->event_asserted = true;
    task->write_gate = CONDUC_STATE;

    // Save QFT bins to task's BASED registry leaf node
    snprintf(task->dat->values[0], sizeof(task->dat->values[0]), "COS:%.2f,SIN:%.2f", cosine_bin, sine_bin);
    
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", task->dat->keys[0], task->dat->values[0]);
    sha256(temp, len, task->dat->node_hash);

    task->ptr.seq++; // Advance sequence

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT FOURIER OVER PROTOCOLS VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *dat_decnet = create_leaf(500, "COS:0.00,SIN:0.00");
    TwoThreeNode *dat_stanag = create_leaf(600, "COS:0.00,SIN:0.00");

    fourier_task_t decnet_task = {
        .device_id = 1,
        .ethertype = ETHERTYPE_DECNET,
        .event_asserted = false,
        .ptr = { .target_key = 500, .seq = 100 },
        .dat = dat_decnet,
        .write_gate = CUTOFF_STATE
    };

    fourier_task_t stanag_task = {
        .device_id = 2,
        .ethertype = ETHERTYPE_STANAG,
        .event_asserted = false,
        .ptr = { .target_key = 600, .seq = 200 },
        .dat = dat_stanag,
        .write_gate = CUTOFF_STATE
    };

    // 1. Process DECnet frame (0x6003) -> Wakes DECnet Fourier, isolates STANAG
    printf("[TEST] Processing DECnet frame (0x6003)...\n");
    fflush(stdout);
    bool ok = fourier_protocol_process(&decnet_task, ETHERTYPE_DECNET, 12.0, -5.0, 2.0, TAG_KERNEL);
    assert(ok == true);
    assert(decnet_task.event_asserted == true);
    assert(decnet_task.write_gate == CONDUC_STATE);
    assert(strcmp(decnet_task.dat->values[0], "COS:12.00,SIN:-5.00") == 0);

    // STANAG remains cutoff
    assert(stanag_task.event_asserted == false);
    assert(stanag_task.write_gate == CUTOFF_STATE);
    printf("   ✓ DECnet Fourier updated successfully. STANAG task remained isolated.\n");
    fflush(stdout);

    // 2. Process STANAG frame (0x8B32) -> Wakes STANAG Fourier, isolates DECnet
    printf("[TEST] Processing STANAG frame (0x8B32)...\n");
    fflush(stdout);
    ok = fourier_protocol_process(&stanag_task, ETHERTYPE_STANAG, 15.0, 3.0, 1.0, TAG_KERNEL);
    assert(ok == true);
    assert(stanag_task.event_asserted == true);
    assert(stanag_task.write_gate == CONDUC_STATE);
    assert(strcmp(stanag_task.dat->values[0], "COS:15.00,SIN:3.00") == 0);
    printf("   ✓ STANAG Fourier updated successfully. DECnet task remained isolated.\n");
    fflush(stdout);

    // 3. Low Q-Factor rejection on DECnet line -> Rejects frame
    printf("[TEST] Processing low Q-factor frame on DECnet line...\n");
    fflush(stdout);
    ok = fourier_protocol_process(&decnet_task, ETHERTYPE_DECNET, 0.1, 0.1, 2.0, TAG_KERNEL);
    assert(ok == false);
    assert(decnet_task.write_gate == CUTOFF_STATE);
    printf("   ✓ Damped signal frame rejected successfully by Q-factor gate.\n");
    fflush(stdout);

    free(dat_decnet);
    free(dat_stanag);
    printf("=============================================================\n");
    printf("FOURIER OVER PROTOCOLS TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
