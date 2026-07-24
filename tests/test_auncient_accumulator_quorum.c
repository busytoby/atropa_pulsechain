#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define MOTZKIN_PRIME 953467954114363ULL
#define COLOR_BLACK    0x000000
#define COLOR_RED      0x0000FF
#define COLOR_CYAN     0xFFFF
#define WARNING_DRONE  110

typedef struct {
    uint64_t accumulator;
    uint32_t border_color;
    uint32_t psg_frequency;
    bool blame_quarantine;
} accumulator_system_t;

// Accumulates validation votes using the non-preferential accumulator formula
static void accumulate_quorum_vote(accumulator_system_t *sys, uint32_t validator_id, uint32_t vote) {
    // A_new = (A_old + validator_id * vote) % MotzkinPrime
    sys->accumulator = (sys->accumulator + ((uint64_t)validator_id * vote)) % MOTZKIN_PRIME;
    printf("   [ACCUMULATE] Validator %u voted %u. Accumulator state: %lu\n", 
           validator_id, vote, sys->accumulator);
    fflush(stdout);
}

// Triggers threshold-based events when the accumulator passes the required consensus threshold
static bool verify_accumulator_threshold(accumulator_system_t *sys, uint64_t required_threshold) {
    if (sys->accumulator >= required_threshold) {
        sys->border_color = COLOR_CYAN;
        sys->psg_frequency = 261;
        printf("   [THRESHOLD PASS] Accumulator (%lu) met quorum threshold (%lu). Consensus committed.\n", 
               sys->accumulator, required_threshold);
        fflush(stdout);
        return true;
    } else {
        sys->border_color = COLOR_RED;
        sys->psg_frequency = WARNING_DRONE;
        sys->blame_quarantine = true;
        printf("   [THRESHOLD FAIL] Accumulator (%lu) below quorum threshold (%lu). Quarantine triggered.\n", 
               sys->accumulator, required_threshold);
        fflush(stdout);
        return false;
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT NON-PREFERENTIAL ACCUMULATOR QUORUM SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    accumulator_system_t sys = {
        .accumulator = 0,
        .border_color = COLOR_BLACK,
        .psg_frequency = 261,
        .blame_quarantine = false
    };

    // Threshold requires at least three validators voting YES (vote = 100)
    // Validator IDs: 1, 2, 3 -> (1 * 100) + (2 * 100) + (3 * 100) = 600
    uint64_t consensus_threshold = 600;

    // 1. Test case: Insufficient votes (only Validator 1 and 2 vote)
    printf("[TEST] Testing insufficient vote accumulation...\n");
    fflush(stdout);
    accumulate_quorum_vote(&sys, 1, 100);
    accumulate_quorum_vote(&sys, 2, 100);
    bool committed = verify_accumulator_threshold(&sys, consensus_threshold);
    assert(committed == false);
    assert(sys.border_color == COLOR_RED);
    assert(sys.blame_quarantine == true);
    printf("   ✓ Blocked insufficient quorum correctly.\n");
    fflush(stdout);

    // Reset system variables
    sys.accumulator = 0;
    sys.border_color = COLOR_BLACK;
    sys.blame_quarantine = false;

    // 2. Test case: Sufficient votes (Validators 1, 2, 3 vote)
    printf("[TEST] Testing sufficient vote accumulation...\n");
    fflush(stdout);
    accumulate_quorum_vote(&sys, 1, 100);
    accumulate_quorum_vote(&sys, 2, 100);
    accumulate_quorum_vote(&sys, 3, 100);
    committed = verify_accumulator_threshold(&sys, consensus_threshold);
    assert(committed == true);
    assert(sys.border_color == COLOR_CYAN);
    assert(sys.blame_quarantine == false);
    printf("   ✓ Approved sufficient quorum successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("ACCUMULATOR QUORUM SUITE COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
