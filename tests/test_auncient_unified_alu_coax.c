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

// Guest VDM Node (The Black SKELETON interface)
typedef struct {
    uint32_t node_id;
    uint32_t curr_heartbeat;
    uint32_t last_heartbeat;
    gate_state_t write_gate;
    char buffer[64];
    uint8_t buffer_hash[HASH_SIZE];
    char backup_val[64];
    uint8_t backup_hash[HASH_SIZE];
} unified_vdm_node_t;

// Exoskeleton Hypervisor State (The Red EXOSKELETON interface)
typedef struct {
    unified_vdm_node_t nodes[3];
    uint32_t coax_token_owner;
    uint8_t supervisor_error_flags[3]; // Active-Low (0 = Safe, 1 = Trap)
} unified_exoskeleton_t;

// Kermit Packet structure over the Coaxial Cable
typedef struct {
    uint8_t mark;
    uint8_t len;
    uint8_t seq;
    char type;
    char data[32];
    uint8_t checksum;
} coax_kermit_packet_t;

// Compute Kermit Checksum
uint8_t coax_kermit_checksum(const coax_kermit_packet_t *pkt) {
    uint32_t sum = pkt->len + pkt->seq + (uint8_t)pkt->type;
    for (int i = 0; i < pkt->len - 3; i++) {
        sum += (uint8_t)pkt->data[i];
    }
    return (uint8_t)((sum + ((sum & 0xC0) >> 6)) & 0x3F);
}

// Helper to compute node hash
static void update_node_hash(unified_vdm_node_t *node) {
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->node_id, node->buffer);
    sha256(temp, len, node->buffer_hash);
}

// -------------------------------------------------------------
// Unified ALU and Coaxial Gating Pipeline
// -------------------------------------------------------------
bool dispatch_unified_coax_frame(unified_exoskeleton_t *exo,
                                 uint32_t sender_id,
                                 uint32_t target_id,
                                 const coax_kermit_packet_t *pkt,
                                 double carrier_freq,
                                 double quality_factor) {
    
    // Default cutoff on target gate
    exo->nodes[target_id].write_gate = CUTOFF_STATE;

    // 1. Back up target state for converse rollback
    strcpy(exo->nodes[target_id].backup_val, exo->nodes[target_id].buffer);
    memcpy(exo->nodes[target_id].backup_hash, exo->nodes[target_id].buffer_hash, HASH_SIZE);

    // 2. EXOSKELETON (Red PNP): Consensus Token check (Sender must own token)
    if (exo->coax_token_owner != sender_id) {
        exo->supervisor_error_flags[sender_id] = 1; // Trap: out of turn write
        return false;
    }

    // 3. EXOSKELETON (Red PNP): Watchdog check (Target guest heartbeat progress)
    if (exo->nodes[target_id].curr_heartbeat == exo->nodes[target_id].last_heartbeat) {
        exo->supervisor_error_flags[target_id] = 1; // Trap: target node hung
        return false;
    }
    // Update watchdog shadow
    exo->nodes[target_id].last_heartbeat = exo->nodes[target_id].curr_heartbeat;

    // 4. SKELETON (Black NPN): Active-high heartbeat liveness check
    if (exo->nodes[target_id].curr_heartbeat == 0) {
        return false; // Stagnant liveness
    }

    // 5. Coaxial Cable: Carrier frequency and quality factor resonance audit (Fourier Gate)
    bool fourier_ok = (carrier_freq == 440.0 && quality_factor >= 0.8);
    if (!fourier_ok) {
        return false; // Signal de-tuned
    }

    // 6. Coaxial Cable: Frame checksum verification (Kermit Gate)
    uint8_t local_check = coax_kermit_checksum(pkt);
    if (pkt->mark != SOH || pkt->checksum != local_check) {
        // Checksum mismatch -> Converse rollback target VDM state
        strcpy(exo->nodes[target_id].buffer, exo->nodes[target_id].backup_val);
        memcpy(exo->nodes[target_id].buffer_hash, exo->nodes[target_id].backup_hash, HASH_SIZE);
        return false;
    }

    // 7. Conduction established: Commit payload to target buffer
    exo->nodes[target_id].write_gate = CONDUC_STATE;
    strcpy(exo->nodes[target_id].buffer, pkt->data);
    update_node_hash(&exo->nodes[target_id]);

    // Rotate consensus token
    exo->coax_token_owner = (sender_id + 1) % 3;

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT UNIFIED ALU AND COAXIAL CABLE VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize hypervisor exoskeleton and guest skeletons
    unified_exoskeleton_t exo = {
        .nodes = {
            { .node_id = 0, .curr_heartbeat = 1, .last_heartbeat = 0, .write_gate = CONDUC_STATE, .buffer = "VAL_0" },
            { .node_id = 1, .curr_heartbeat = 1, .last_heartbeat = 0, .write_gate = CONDUC_STATE, .buffer = "VAL_1" },
            { .node_id = 2, .curr_heartbeat = 1, .last_heartbeat = 0, .write_gate = CONDUC_STATE, .buffer = "VAL_2" }
        },
        .coax_token_owner = 0, // Node 0 starts with token
        .supervisor_error_flags = { 0, 0, 0 } // No error traps
    };

    // Calculate initial Merkle hashes
    for (int i = 0; i < 3; i++) {
        update_node_hash(&exo.nodes[i]);
    }

    // Define a compliant Kermit packet
    coax_kermit_packet_t valid_packet = {
        .mark = SOH,
        .len = 16,
        .seq = 1,
        .type = 'D',
        .data = "DATA_BLOCK_UNION"
    };
    valid_packet.checksum = coax_kermit_checksum(&valid_packet);

    // 1. Dispatch compliant frame from Node 0 to Node 1 -> Should succeed
    printf("[TEST] Dispatching compliant frame Node 0 -> Node 1...\n");
    fflush(stdout);
    
    // Advance heartbeats to satisfy liveness watchdogs
    exo.nodes[1].curr_heartbeat = 2;
    
    bool ok = dispatch_unified_coax_frame(&exo, 0, 1, &valid_packet, 440.0, 0.85);
    assert(ok == true);
    assert(exo.nodes[1].write_gate == CONDUC_STATE);
    assert(strcmp(exo.nodes[1].buffer, "DATA_BLOCK_UNION") == 0);
    assert(exo.coax_token_owner == 1); // Token rotated to Node 1
    printf("   ✓ Conduction established. Packet committed and token rotated.\n");
    fflush(stdout);

    // 2. Out-of-turn write attempt from Node 2 without token -> Should block
    printf("[TEST] Node 2 attempts transmission out of turn...\n");
    fflush(stdout);
    exo.nodes[0].curr_heartbeat = 3;
    ok = dispatch_unified_coax_frame(&exo, 2, 0, &valid_packet, 440.0, 0.85);
    assert(ok == false);
    assert(exo.supervisor_error_flags[2] == 1); // Node 2 trapped
    printf("   ✓ Out-of-turn write blocked. Exoskeleton trapped Node 2.\n");
    fflush(stdout);

    // 3. Dispatch corrupted frame (bad checksum) -> Should rollback target state
    printf("[TEST] Dispatching corrupted frame (checksum mismatch)...\n");
    fflush(stdout);
    coax_kermit_packet_t bad_packet = valid_packet;
    bad_packet.checksum = 99; // Corrupted

    // Advance target Node 2 heartbeat
    exo.nodes[2].curr_heartbeat = 2;
    
    // Node 1 holds token, attempts to write to Node 2
    uint8_t hash_before[HASH_SIZE];
    memcpy(hash_before, exo.nodes[2].buffer_hash, HASH_SIZE);
    
    ok = dispatch_unified_coax_frame(&exo, 1, 2, &bad_packet, 440.0, 0.85);
    assert(ok == false);
    assert(exo.nodes[2].write_gate == CUTOFF_STATE);
    assert(strcmp(exo.nodes[2].buffer, "VAL_2") == 0); // Preserved
    assert(memcmp(hash_before, exo.nodes[2].buffer_hash, HASH_SIZE) == 0); // Hash preserved
    printf("   ✓ Corrupted frame dropped. Target Node 2 state rolled back.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("UNIFIED ALU AND COAXIAL CABLE TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
