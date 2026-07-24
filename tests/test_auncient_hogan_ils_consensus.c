#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define LANES 4
#define COLOR_BLACK    0x000000
#define COLOR_RED      0x0000FF
#define COLOR_CYAN     0xFFFF
#define WARNING_DRONE  110
#define REQUIRED_QUORUM_MASK 0x07

typedef struct {
    int64_t max_interest_ceiling;
    uint32_t required_validator_mask; // SGPR consensus parameter
    uint32_t baseline_color;          // Black SGPR base
} sgpr_bank_t;

typedef struct {
    uint32_t account_id[LANES];
    int64_t loan_amount[LANES];
    int64_t loan_duration_months[LANES];
    uint32_t collected_validator_mask[LANES]; // Vector signature bits
    uint32_t status[LANES];                  // Red VGPR alert indicator
} vgpr_bank_t;

typedef struct {
    uint32_t account_id;
    int64_t balance;
} hogan_account_t;

typedef struct {
    uint32_t border_color;
    uint32_t psg_frequency;
    bool blame_quarantine;
} huc_ocean_system_t;

static void process_ils_consensus(huc_ocean_system_t *huc,
                                  const sgpr_bank_t *sgprs,
                                  vgpr_bank_t *vgprs,
                                  hogan_account_t *registry) {
    printf("[PROCESS] Executing Hogan ILS Loan processing with consensus quorums...\n");
    fflush(stdout);

    for (int lane = 0; lane < LANES; lane++) {
        uint32_t acc = vgprs->account_id[lane];
        int64_t amt = vgprs->loan_amount[lane];
        int64_t duration = vgprs->loan_duration_months[lane];
        uint32_t sig_mask = vgprs->collected_validator_mask[lane];
        
        // Default to black SGPR baseline color
        huc->border_color = sgprs->baseline_color;

        // 1. Calculate and Verify Interest Ceiling
        int64_t interest = (amt * duration * 5) / 100;
        if (interest > sgprs->max_interest_ceiling) {
            vgprs->status[lane] = COLOR_RED;
            huc->border_color = COLOR_RED; // Red VGPR alert color
            huc->psg_frequency = WARNING_DRONE;
            huc->blame_quarantine = true;
            printf("   [ILS FAIL] Lane %d: Account %d interest payout (%ld) exceeds ceiling (%ld)! Red VGPR alert activated.\n", 
                   lane, acc, interest, sgprs->max_interest_ceiling);
            fflush(stdout);
            break;
        }

        // 2. Verify Consensus Quorum Signatures
        if ((sig_mask & sgprs->required_validator_mask) != sgprs->required_validator_mask) {
            vgprs->status[lane] = COLOR_RED;
            huc->border_color = COLOR_RED; // Red VGPR alert color
            huc->psg_frequency = WARNING_DRONE;
            huc->blame_quarantine = true;
            printf("   [ILS FAIL] Lane %d: Account %d lacks sufficient quorum signatures (Mask: 0x%02X < 0x%02X)! Red VGPR alert activated.\n", 
                   lane, acc, sig_mask, sgprs->required_validator_mask);
            fflush(stdout);
            break;
        }

        // Success: Credit loan amount to account balance
        registry[acc].balance += amt;
        vgprs->status[lane] = COLOR_CYAN;
        huc->border_color = COLOR_CYAN;
        huc->psg_frequency = 329; // Success note E4
        printf("   [ILS PASS] Lane %d: Approved Loan of %ld to Account %d (Interest: %ld, Signatures: 0x%02X). New Balance: %ld.\n", 
               lane, amt, acc, interest, sig_mask, registry[acc].balance);
        fflush(stdout);
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT HOGAN ILS LOAN CONSENSUS VERIFICATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize mock Hogan accounts
    hogan_account_t registry[100];
    memset(registry, 0, sizeof(registry));
    registry[10].account_id = 10;
    registry[10].balance = 5000;
    registry[20].account_id = 20;
    registry[20].balance = 1200;

    // SGPR settings
    sgpr_bank_t sgprs = {
        .max_interest_ceiling = 500,
        .required_validator_mask = REQUIRED_QUORUM_MASK,
        .baseline_color = COLOR_BLACK
    };

    // VGPR settings
    vgpr_bank_t vgprs = {
        .account_id = { 10, 20, 10, 20 },
        .loan_amount = { 1000, 1000, 15000, 100 },
        .loan_duration_months = { 12, 12, 12, 12 },
        .collected_validator_mask = {
            0x07, // Lane 0: Pass (Interest: 600? Wait, 1000 * 12 * 5 / 100 = 600 > 500? Fails interest check!)
            0x07, 
            0x07, 
            0x07
        },
        .status = { 0 }
    };

    // Adjust lane 0 loan amount so it passes interest check: 1000 -> 500 (Interest: 500 * 12 * 5 / 100 = 300 <= 500)
    vgprs.loan_amount[0] = 500;
    // Lane 1 has loan amount 1000, duration 12 -> Interest: 600 > 500. Expected fail.

    huc_ocean_system_t huc = {
        .border_color = COLOR_BLACK,
        .psg_frequency = 261,
        .blame_quarantine = false
    };

    // Process ILS loan checks
    process_ils_consensus(&huc, &sgprs, &vgprs, registry);

    // Verify assertions
    printf("[TEST] Verifying combined system states...\n");
    fflush(stdout);
    
    // Lane 0: Approved (Interest: 300 <= 500, Signatures: 0x07 == 0x07)
    assert(vgprs.status[0] == COLOR_CYAN);
    
    // Lane 1: Fail (Interest: 600 > 500)
    assert(vgprs.status[1] == COLOR_RED);
    assert(huc.border_color == COLOR_RED);
    assert(huc.psg_frequency == WARNING_DRONE);
    assert(huc.blame_quarantine == true);
    
    printf("   ✓ ILS consensus quorum and register alerts verified successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("HOGAN ILS LOAN CONSENSUS VERIFICATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
