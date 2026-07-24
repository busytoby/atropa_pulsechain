#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define SOH 0x01

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

// Simulated .dat.bin Merkle Node
typedef struct {
    uint32_t key;
    char value[64];
    uint8_t hash[HASH_SIZE];
} dat_bin_node_t;

// Guest context with .dat.bin database slice
typedef struct {
    uint32_t node_id;
    dat_bin_node_t database_slice;
    gate_state_t write_gate;
    char backup_val[64];
    uint8_t backup_hash[HASH_SIZE];
} dat_bin_vdm_t;

// Hypervisor Exoskeleton State
typedef struct {
    dat_bin_vdm_t guests[3];
    uint32_t global_token_owner;
} dat_bin_exoskeleton_t;

// AUTODIN transaction frame
typedef struct {
    uint8_t mark;
    uint8_t len;
    uint8_t seq;
    char type;
    char data[32];
    uint8_t checksum;
} autodin_frame_t;

// Compute simple checksum
uint8_t autodin_calc_checksum(const autodin_frame_t *pkt) {
    uint32_t sum = pkt->len + pkt->seq + (uint8_t)pkt->type;
    for (int i = 0; i < pkt->len - 3; i++) {
        sum += (uint8_t)pkt->data[i];
    }
    return (uint8_t)((sum + ((sum & 0xC0) >> 6)) & 0x3F);
}

// Compute Merkle node hash
static void update_dat_bin_hash(dat_bin_node_t *node) {
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->key, node->value);
    sha256(temp, len, node->hash);
}

// -------------------------------------------------------------
// AUTODIN Ingestion and APDL .dat.bin Commitment Loop
// -------------------------------------------------------------
bool autodin_ingest_transaction(dat_bin_exoskeleton_t *exo,
                                uint32_t sender_id,
                                uint32_t target_id,
                                const autodin_frame_t *frame,
                                double freq,
                                double q_factor) {
    dat_bin_vdm_t *target = &exo->guests[target_id];
    target->write_gate = CUTOFF_STATE;

    // 1. Back up database state for converse rollback
    strcpy(target->backup_val, target->database_slice.value);
    memcpy(target->backup_hash, target->database_slice.hash, HASH_SIZE);

    // 2. Precondition check: Token consensus verification
    if (exo->global_token_owner != sender_id) {
        return false; // Out-of-turn write blocked
    }

    // 3. Precondition check: Fourier carrier resonance verification
    bool phi_fourier = (freq == 440.0 && q_factor >= 0.8);
    if (!phi_fourier) {
        return false; // Signal mismatch
    }

    // 4. Update .dat.bin state
    strcpy(target->database_slice.value, frame->data);
    update_dat_bin_hash(&target->database_slice);

    // 5. Postcondition check: Checksum and packet mark verification (Kermit Gate)
    uint8_t local_check = autodin_calc_checksum(frame);
    bool psi_clearance = (frame->mark == SOH && frame->checksum == local_check);
    if (!psi_clearance) {
        // Rollback target database node values and hashes using converse shadow logs
        strcpy(target->database_slice.value, target->backup_val);
        memcpy(target->database_slice.hash, target->backup_hash, HASH_SIZE);
        return false;
    }

    // Pass: establish conduction
    target->write_gate = CONDUC_STATE;
    
    // Rotate token ring
    exo->global_token_owner = (sender_id + 1) % 3;
    
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT AUTODIN APDL .DAT.BIN TRANSACTION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize database slices
    dat_bin_exoskeleton_t exo = {
        .guests = {
            { .node_id = 0, .database_slice = { .key = 100, .value = "STATE_0" }, .write_gate = CONDUC_STATE },
            { .node_id = 1, .database_slice = { .key = 200, .value = "STATE_1" }, .write_gate = CONDUC_STATE },
            { .node_id = 2, .database_slice = { .key = 300, .value = "STATE_2" }, .write_gate = CONDUC_STATE }
        },
        .global_token_owner = 0
    };

    for (int i = 0; i < 3; i++) {
        update_dat_bin_hash(&exo.guests[i].database_slice);
    }

    // Define compliant AUTODIN transaction frame
    autodin_frame_t valid_frame = {
        .mark = SOH,
        .len = 16,
        .seq = 1,
        .type = 'T',
        .data = "TX_COMMIT_VAL_01"
    };
    valid_frame.checksum = autodin_calc_checksum(&valid_frame);

    // 1. Ingest compliant AUTODIN frame to .dat.bin database -> Should succeed
    printf("[TEST] Ingesting compliant AUTODIN frame...\n");
    fflush(stdout);
    bool ok = autodin_ingest_transaction(&exo, 0, 1, &valid_frame, 440.0, 0.85);
    assert(ok == true);
    assert(exo.guests[1].write_gate == CONDUC_STATE);
    assert(strcmp(exo.guests[1].database_slice.value, "TX_COMMIT_VAL_01") == 0);
    assert(exo.global_token_owner == 1);
    printf("   ✓ Transaction committed. .dat.bin Merkle hash updated.\n");
    fflush(stdout);

    uint8_t hash_before[HASH_SIZE];
    memcpy(hash_before, exo.guests[2].database_slice.hash, HASH_SIZE);

    // 2. Ingest corrupted AUTODIN frame -> Should rollback .dat.bin database state
    printf("[TEST] Ingesting corrupted AUTODIN frame...\n");
    fflush(stdout);
    autodin_frame_t bad_frame = valid_frame;
    bad_frame.checksum = 88; // Corrupted
    
    // Node 1 holds token, attempts to write to Node 2
    ok = autodin_ingest_transaction(&exo, 1, 2, &bad_frame, 440.0, 0.85);
    assert(ok == false);
    assert(exo.guests[2].write_gate == CUTOFF_STATE);
    assert(strcmp(exo.guests[2].database_slice.value, "STATE_2") == 0); // Restored
    assert(memcmp(hash_before, exo.guests[2].database_slice.hash, HASH_SIZE) == 0); // Hash restored
    printf("   ✓ Mismatch caught. .dat.bin state rolled back successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("AUTODIN APDL .DAT.BIN TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
