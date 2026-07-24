#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define LANES 4
#define COLOR_BLACK    0x000000 // Black SGPR base constraint
#define COLOR_RED      0x0000FF // Red VGPR alert color
#define COLOR_CYAN     0xFFFF
#define WARNING_DRONE  110

typedef enum {
    TYPE_ACH = 1,
    TYPE_LOAN = 2
} entry_type_t;

typedef struct {
    int64_t reserve_limit;
    int64_t max_interest_ceiling; // SGPR parameter
    uint32_t baseline_color;      // Black SGPR base color
} sgpr_bank_v2_t;

typedef struct {
    uint32_t account_id[LANES];
    entry_type_t entry_type[LANES];
    int64_t amount[LANES];
    int64_t loan_duration_months[LANES];
    uint32_t status_output[LANES]; // Red VGPR alert indicator
} vgpr_bank_v2_t;

typedef struct {
    uint32_t account_id;
    int64_t balance;
} hogan_account_t;

typedef struct {
    uint32_t border_color;
    uint32_t psg_frequency;
    bool blame_quarantine;
} huc_ocean_system_v2_t;

static void process_hogan_ach_loans(huc_ocean_system_v2_t *huc,
                                    const sgpr_bank_v2_t *sgprs,
                                    vgpr_bank_v2_t *vgprs,
                                    hogan_account_t *registry) {
    printf("[PROCESS] Executing Hogan ACH/Loan batch under Ocean Soft loader protocols...\n");
    fflush(stdout);

    for (int lane = 0; lane < LANES; lane++) {
        uint32_t acc = vgprs->account_id[lane];
        entry_type_t type = vgprs->entry_type[lane];
        int64_t amt = vgprs->amount[lane];
        
        // Default to black SGPR baseline color
        huc->border_color = sgprs->baseline_color;

        if (type == TYPE_ACH) {
            // Check reserve limit (DDO ACH validation)
            int64_t projected_balance = registry[acc].balance - amt;
            if (projected_balance >= sgprs->reserve_limit) {
                registry[acc].balance -= amt;
                vgprs->status_output[lane] = COLOR_CYAN;
                huc->border_color = COLOR_CYAN;
                huc->psg_frequency = 261;
                printf("   [ACH PASS] Lane %d: Ingested ACH. Account %d balance: %ld.\n", 
                       lane, acc, registry[acc].balance);
                fflush(stdout);
            } else {
                vgprs->status_output[lane] = COLOR_RED;
                huc->border_color = COLOR_RED;
                huc->psg_frequency = WARNING_DRONE;
                huc->blame_quarantine = true;
                printf("   [ACH FAIL] Lane %d: Reserve sweep violation! Red VGPR alert activated.\n", lane);
                fflush(stdout);
                break;
            }
        } else if (type == TYPE_LOAN) {
            // Calculate interest: simple interest calculation for this lane
            int64_t duration = vgprs->loan_duration_months[lane];
            int64_t interest = (amt * duration * 5) / 100; // 5% annual rate simplified
            
            if (interest <= sgprs->max_interest_ceiling) {
                registry[acc].balance += amt;
                vgprs->status_output[lane] = COLOR_CYAN;
                huc->border_color = COLOR_CYAN;
                huc->psg_frequency = 329; // Success note E4
                printf("   [LOAN PASS] Lane %d: Approved Loan of %ld to Account %d (Interest: %ld).\n", 
                       lane, amt, acc, interest);
                fflush(stdout);
            } else {
                vgprs->status_output[lane] = COLOR_RED;
                huc->border_color = COLOR_RED;
                huc->psg_frequency = WARNING_DRONE;
                huc->blame_quarantine = true;
                printf("   [LOAN FAIL] Lane %d: Interest ceiling violation (%ld > %ld)! Red VGPR alert activated.\n", 
                       lane, interest, sgprs->max_interest_ceiling);
                fflush(stdout);
                break;
            }
        }
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT HOGAN-HUDSON ACH/LOAN OCEAN INTEGRATION LEVEL 2\n");
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
    sgpr_bank_v2_t sgprs = {
        .reserve_limit = 1000,
        .max_interest_ceiling = 500, // Maximum allowed interest payout in epoch
        .baseline_color = COLOR_BLACK
    };

    // VGPR settings
    vgpr_bank_v2_t vgprs = {
        .account_id = { 10, 20, 10, 20 },
        .entry_type = { TYPE_ACH, TYPE_LOAN, TYPE_LOAN, TYPE_ACH },
        .amount = { 1500, 1000, 15000, 100 }, // Lane 2 interest: 15000 * 12 * 5 / 100 = 9000 > 500 (Fail interest check)
        .loan_duration_months = { 0, 12, 12, 0 },
        .status_output = { 0 }
    };

    huc_ocean_system_v2_t huc = {
        .border_color = COLOR_BLACK,
        .psg_frequency = 261,
        .blame_quarantine = false
    };

    // Process
    process_hogan_ach_loans(&huc, &sgprs, &vgprs, registry);

    // Verify assertions
    printf("[TEST] Verifying combined system states...\n");
    fflush(stdout);
    
    // Lane 0: ACH 1500. 5000 - 1500 = 3500 >= 1000 (Pass)
    assert(vgprs.status_output[0] == COLOR_CYAN);
    
    // Lane 1: Loan 1000, 12 months. Interest: 1000 * 12 * 5 / 100 = 600. 
    // Wait, 600 > 500 max_interest_ceiling! It should fail!
    // Ah, yes! 600 is greater than 500, so Lane 1 should trigger the Fail and abort.
    assert(vgprs.status_output[1] == COLOR_RED);
    assert(huc.border_color == COLOR_RED);
    assert(huc.psg_frequency == WARNING_DRONE);
    assert(huc.blame_quarantine == true);
    
    printf("   ✓ System registers and peripheral alerts verified successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("HOGAN-HUDSON ACH/LOAN TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
