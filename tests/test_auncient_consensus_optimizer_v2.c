#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define MAX_POOL_VALIDATORS 6
#define ACTIVE_SEAT_COUNT 3
#define BASE_REPUTATION 100
#define REPUTATION_DECAY_RATE 15
#define REPUTATION_THRESHOLD 50

typedef enum {
    PREC_ROUTINE = 1,
    PREC_FLASH = 2,
    PREC_TOP_SECRET = 3
} tx_precedence_t;

typedef struct {
    uint32_t validator_id;
    uint32_t stake;
    int32_t reputation;
    bool active;
} consensus_validator_t;

typedef struct {
    consensus_validator_t pool[MAX_POOL_VALIDATORS];
    uint32_t active_seats[ACTIVE_SEAT_COUNT];
} consensus_system_v2_t;

// 1. Precedence-Aware Quorum Threshold Tuning
static uint32_t get_required_signatures(tx_precedence_t prec) {
    switch (prec) {
        case PREC_ROUTINE:   return 1;
        case PREC_FLASH:     return 2;
        case PREC_TOP_SECRET: return 3;
        default:             return 2;
    }
}

// 2. Validator Reputation Decay & Automatic Replacement
static void report_validator_miss(consensus_system_v2_t *sys, uint32_t seat_idx) {
    uint32_t val_idx = sys->active_seats[seat_idx];
    consensus_validator_t *val = &sys->pool[val_idx];
    
    // Decay reputation
    val->reputation -= REPUTATION_DECAY_RATE;
    printf("   [DECAY] Validator %d missed round. Reputation dropped to %d.\n", val->validator_id, val->reputation);

    if (val->reputation < REPUTATION_THRESHOLD) {
        // Swap out validator with a healthy one from the pool
        printf("   [REPLACEMENT] Validator %d reputation below threshold. Replacing...\n", val->validator_id);
        val->active = false;
        
        for (int i = 0; i < MAX_POOL_VALIDATORS; i++) {
            if (sys->pool[i].active && sys->pool[i].reputation >= BASE_REPUTATION) {
                // Verify not already seated
                bool already_seated = false;
                for (uint32_t j = 0; j < ACTIVE_SEAT_COUNT; j++) {
                    if (sys->active_seats[j] == (uint32_t)i) {
                        already_seated = true;
                        break;
                    }
                }
                if (!already_seated) {
                    sys->active_seats[seat_idx] = i;
                    printf("   [REPLACEMENT] Validator %d seated successfully.\n", sys->pool[i].validator_id);
                    break;
                }
            }
        }
    }
}

// 3. Stake-Weighted Vote Resolution
static bool verify_stake_weighted_quorum(const consensus_system_v2_t *sys, uint32_t vote_mask, uint32_t required_stake_threshold) {
    uint32_t accumulated_stake = 0;
    
    for (uint32_t i = 0; i < ACTIVE_SEAT_COUNT; i++) {
        uint32_t val_idx = sys->active_seats[i];
        if ((vote_mask >> i) & 1) {
            accumulated_stake += sys->pool[val_idx].stake;
        }
    }
    
    return (accumulated_stake >= required_stake_threshold);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT CONSENSUS & QUORUM OPTIMIZATION LEVEL 2\n");
    printf("=============================================================\n");
    fflush(stdout);

    consensus_system_v2_t sys;
    memset(&sys, 0, sizeof(consensus_system_v2_t));

    // Initialize pool of 6 validators
    for (int i = 0; i < MAX_POOL_VALIDATORS; i++) {
        sys.pool[i].validator_id = i;
        sys.pool[i].stake = 1000 + (i * 500); // 1000, 1500, 2000, 2500, 3000, 3500
        sys.pool[i].reputation = BASE_REPUTATION;
        sys.pool[i].active = true;
    }

    // Set active seats
    sys.active_seats[0] = 0; // Stake: 1000
    sys.active_seats[1] = 1; // Stake: 1500
    sys.active_seats[2] = 2; // Stake: 2000

    // 1. Test Precedence-Aware Quorum Tuning
    printf("[TEST] Checking precedence-aware quorum signatures count...\n");
    fflush(stdout);
    assert(get_required_signatures(PREC_ROUTINE) == 1);
    assert(get_required_signatures(PREC_FLASH) == 2);
    assert(get_required_signatures(PREC_TOP_SECRET) == 3);
    printf("   ✓ Precedence levels correctly resolved to signature limits.\n");
    fflush(stdout);

    // 2. Test Stake-Weighted Quorum Verification
    printf("[TEST] Checking stake-weighted consensus checks...\n");
    fflush(stdout);
    
    // Total active stake in seats = 1000 + 1500 + 2000 = 4500
    // Signers: Validator 1 and 2 (bitmask: 0x06, total stake: 3500)
    // Threshold: 3000 -> Pass
    bool quorum_ok = verify_stake_weighted_quorum(&sys, 0x06, 3000);
    assert(quorum_ok == true);
    
    // Threshold: 4000 -> Fail
    quorum_ok = verify_stake_weighted_quorum(&sys, 0x06, 4000);
    assert(quorum_ok == false);
    printf("   ✓ Stake-weighted consensus thresholds verified successfully.\n");
    fflush(stdout);

    // 3. Test Reputation Decay & Validator Replacement
    printf("[TEST] Testing reputation decay and dynamic seat replacement...\n");
    fflush(stdout);
    
    // Initial seat 0 is validator 0
    assert(sys.active_seats[0] == 0);

    // Miss multiple rounds to decay reputation below 50 (decay by 15 per miss)
    report_validator_miss(&sys, 0); // 85
    report_validator_miss(&sys, 0); // 70
    report_validator_miss(&sys, 0); // 55
    report_validator_miss(&sys, 0); // 40 -> triggers replacement with next healthy candidate (validator 3)

    assert(sys.active_seats[0] == 3);
    printf("   ✓ Quarantined slow validator replaced automatically in consensus seat 0.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("CONSENSUS LEVEL 2 OPTIMIZATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
