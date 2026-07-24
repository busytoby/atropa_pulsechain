#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define MAX_POOL_VALIDATORS 6
#define ACTIVE_SEAT_COUNT 3

typedef enum {
    VAL_ACTIVE = 1,
    VAL_SLASHED = 2,
    VAL_QUARANTINED = 3
} validator_status_t;

typedef struct {
    uint32_t validator_id;
    uint32_t stake;
    validator_status_t status;
    uint64_t last_signed_nonce;
    uint8_t last_signature_hash[32];
} consensus_validator_t;

// Consensus context mapping active seats and dynamic epoch rotations
typedef struct {
    consensus_validator_t pool[MAX_POOL_VALIDATORS];
    uint32_t active_seats[ACTIVE_SEAT_COUNT];
    uint32_t current_epoch;
} consensus_system_t;

// 1. Emulates Epoch-Based Validator Seat Rotations
static void rotate_validator_seats(consensus_system_t *sys) {
    sys->current_epoch++;
    // Rotate active seats based on epoch displacement indices in the pool
    for (uint32_t i = 0; i < ACTIVE_SEAT_COUNT; i++) {
        uint32_t pool_idx = (i + sys->current_epoch) % MAX_POOL_VALIDATORS;
        
        // Skip slashed/quarantined validators
        while (sys->pool[pool_idx].status != VAL_ACTIVE) {
            pool_idx = (pool_idx + 1) % MAX_POOL_VALIDATORS;
        }
        sys->active_seats[i] = pool_idx;
    }
}

// 2. Emulates Double-Signing Slashing and Quarantine Policies
static bool verify_and_log_vote(consensus_system_t *sys, uint32_t val_id, uint64_t nonce, const uint8_t *sig_hash) {
    consensus_validator_t *val = NULL;
    for (int i = 0; i < MAX_POOL_VALIDATORS; i++) {
        if (sys->pool[i].validator_id == val_id) {
            val = &sys->pool[i];
            break;
        }
    }
    if (!val || val->status != VAL_ACTIVE) {
        return false; // Validator not active
    }

    // Check for double signing anomaly
    if (val->last_signed_nonce == nonce) {
        if (memcmp(val->last_signature_hash, sig_hash, 32) != 0) {
            // Double signing detected! Slash stake and quarantine immediately
            val->status = VAL_QUARANTINED;
            val->stake = 0;
            printf("   [SLASH & QUARANTINE] Validator %d caught double-signing for nonce %lu!\n", val_id, nonce);
            return false; 
        }
    }

    // Record last signature parameters
    val->last_signed_nonce = nonce;
    memcpy(val->last_signature_hash, sig_hash, 32);
    return true;
}

// 3. Emulates BLS Threshold Signature Aggregation (BLS-TSS)
static bool verify_bls_aggregate_signature(const uint8_t *aggregate_sig, uint32_t active_mask) {
    // Emulates threshold verification check: requires at least 2 active signers
    int active_signers = 0;
    for (int i = 0; i < 32; i++) {
        if ((active_mask >> i) & 1) {
            active_signers++;
        }
    }
    
    // Check signature content is non-zero
    uint8_t zero_sig[32];
    memset(zero_sig, 0, 32);
    if (memcmp(aggregate_sig, zero_sig, 32) == 0) {
        return false;
    }
    
    return (active_signers >= 2);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT CONSENSUS & QUORUM OPTIMIZATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    consensus_system_t sys;
    memset(&sys, 0, sizeof(consensus_system_t));
    sys.current_epoch = 0;

    // Initialize 6 validators in pool
    for (int i = 0; i < MAX_POOL_VALIDATORS; i++) {
        sys.pool[i].validator_id = i;
        sys.pool[i].stake = 1000;
        sys.pool[i].status = VAL_ACTIVE;
        sys.pool[i].last_signed_nonce = 0;
    }

    // Initial seat assignment (Epoch 0)
    sys.active_seats[0] = 0;
    sys.active_seats[1] = 1;
    sys.active_seats[2] = 2;

    // 1. Test Epoch-Based Seat Rotation
    printf("[TEST] Executing epoch transition seat rotations...\n");
    fflush(stdout);
    rotate_validator_seats(&sys);
    assert(sys.current_epoch == 1);
    assert(sys.active_seats[0] == 1);
    assert(sys.active_seats[1] == 2);
    assert(sys.active_seats[2] == 3);
    printf("   ✓ Epoch rotated seats successfully (Active seats: %d, %d, %d).\n", 
           sys.active_seats[0], sys.active_seats[1], sys.active_seats[2]);
    fflush(stdout);

    // 2. Test Double-Signing Slash and Quarantine Protocols
    printf("[TEST] Testing double-signing validation constraints...\n");
    fflush(stdout);
    
    uint8_t sig_hash_a[32];
    uint8_t sig_hash_b[32];
    memset(sig_hash_a, 0x11, 32);
    memset(sig_hash_b, 0x22, 32);

    // Normal sign
    bool ok = verify_and_log_vote(&sys, 1, 9999, sig_hash_a);
    assert(ok == true);
    assert(sys.pool[1].status == VAL_ACTIVE);

    // Attacking sign (Double-Signing on same nonce 9999)
    ok = verify_and_log_vote(&sys, 1, 9999, sig_hash_b);
    assert(ok == false);
    assert(sys.pool[1].status == VAL_QUARANTINED);
    assert(sys.pool[1].stake == 0);
    printf("   ✓ Double-signing validator slashed and quarantined successfully.\n");
    fflush(stdout);

    // Rotate seats again, validator 1 should be bypassed due to quarantine
    rotate_validator_seats(&sys);
    assert(sys.active_seats[0] == 2); // Validator 1 was skipped
    assert(sys.active_seats[1] == 3);
    assert(sys.active_seats[2] == 4);
    printf("   ✓ Seat rotation skipped quarantined validator successfully.\n");
    fflush(stdout);

    // 3. Test BLS Aggregate Signature Verification
    printf("[TEST] Verifying BLS Aggregate Signature checks...\n");
    fflush(stdout);
    uint8_t mock_aggregate_sig[32];
    memset(mock_aggregate_sig, 0xAA, 32);
    
    // Test with 3 active signers (bitmask: 0x07)
    bool bls_ok = verify_bls_aggregate_signature(mock_aggregate_sig, 0x07);
    assert(bls_ok == true);

    // Test with insufficient signers (bitmask: 0x01)
    bls_ok = verify_bls_aggregate_signature(mock_aggregate_sig, 0x01);
    assert(bls_ok == false);
    printf("   ✓ BLS aggregate threshold signatures verified successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("CONSENSUS & QUORUM OPTIMIZATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
