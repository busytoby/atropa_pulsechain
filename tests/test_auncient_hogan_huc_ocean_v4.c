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

typedef struct {
    int64_t sweep_ceiling; // Threshold above which funds are swept
    int64_t sweep_reserve; // Target balance to leave in DDA
    uint32_t baseline_color; // Black SGPR base
} sgpr_bank_v4_t;

typedef struct {
    uint32_t dda_account_id[LANES];
    int64_t balance[LANES];
    uint32_t savings_account_id[LANES];
    uint32_t status[LANES]; // Red VGPR alert indicator
} vgpr_bank_v4_t;

typedef struct {
    uint32_t account_id;
    int64_t balance;
} hogan_account_t;

typedef struct {
    uint32_t border_color;
    uint32_t psg_frequency;
    bool blame_quarantine;
} huc_ocean_system_v4_t;

static void process_hogan_sweep_rules(huc_ocean_system_v4_t *huc,
                                      const sgpr_bank_v4_t *sgprs,
                                      vgpr_bank_v4_t *vgprs,
                                      hogan_account_t *registry) {
    printf("[PROCESS] Executing Hogan DDO Sweep rules under Ocean Soft protocols...\n");
    fflush(stdout);

    for (int lane = 0; lane < LANES; lane++) {
        uint32_t dda = vgprs->dda_account_id[lane];
        uint32_t sav = vgprs->savings_account_id[lane];
        int64_t bal = vgprs->balance[lane];
        
        // Default to black SGPR baseline color
        huc->border_color = sgprs->baseline_color;

        // Verify account registry limits
        if (registry[dda].account_id == 0 || registry[sav].account_id == 0) {
            vgprs->status[lane] = COLOR_RED;
            huc->border_color = COLOR_RED;
            huc->psg_frequency = WARNING_DRONE;
            huc->blame_quarantine = true;
            printf("   [SWEEP FAIL] Lane %d: Unregistered accounts! Red VGPR alert activated.\n", lane);
            fflush(stdout);
            break;
        }

        // Apply sweep rule if balance exceeds ceiling (SGPR sweep ceiling parameter)
        if (bal > sgprs->sweep_ceiling) {
            int64_t sweep_amt = bal - sgprs->sweep_reserve;
            
            if (sweep_amt > 0) {
                // Perform transfer
                registry[dda].balance -= sweep_amt;
                registry[sav].balance += sweep_amt;
                vgprs->status[lane] = COLOR_CYAN;
                huc->border_color = COLOR_CYAN;
                huc->psg_frequency = 261;
                printf("   [SWEEP PASS] Lane %d: Swept %ld from Account %d to Account %d.\n", 
                       lane, sweep_amt, dda, sav);
                fflush(stdout);
            } else {
                vgprs->status[lane] = COLOR_RED;
                huc->border_color = COLOR_RED;
                huc->psg_frequency = WARNING_DRONE;
                huc->blame_quarantine = true;
                printf("   [SWEEP FAIL] Lane %d: Sweep logic error! Red VGPR alert activated.\n", lane);
                fflush(stdout);
                break;
            }
        } else {
            // Under ceiling: no sweep needed, keep baseline color
            vgprs->status[lane] = COLOR_BLACK;
            printf("   [SWEEP SKIP] Lane %d: Account %d balance (%ld) below sweep ceiling.\n", 
                   lane, dda, bal);
            fflush(stdout);
        }
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT HOGAN-HUDSON OCEAN INTEGRATION LEVEL 4\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize mock Hogan accounts
    hogan_account_t registry[100];
    memset(registry, 0, sizeof(registry));
    registry[10].account_id = 10;
    registry[10].balance = 15000;
    registry[20].account_id = 20;
    registry[20].balance = 5000;
    registry[30].account_id = 30;
    registry[30].balance = 2000;

    // SGPR settings: Black base background color, sweep ceiling at 10,000, leave 5,000 reserve
    sgpr_bank_v4_t sgprs = {
        .sweep_ceiling = 10000,
        .sweep_reserve = 5000,
        .baseline_color = COLOR_BLACK
    };

    // VGPR settings
    vgpr_bank_v4_t vgprs = {
        .dda_account_id = { 10, 30, 99, 10 }, // Lane 2 has unregistered account 99
        .balance = { 15000, 2000, 12000, 15000 },
        .savings_account_id = { 20, 20, 20, 20 },
        .status = { 0 }
    };

    huc_ocean_system_v4_t huc = {
        .border_color = COLOR_BLACK,
        .psg_frequency = 261,
        .blame_quarantine = false
    };

    // Process sweep rules
    process_hogan_sweep_rules(&huc, &sgprs, &vgprs, registry);

    // Verify assertions
    printf("[TEST] Verifying combined system states...\n");
    fflush(stdout);
    
    // Lane 0: Balance 15,000 > 10,000. Sweep: 15,000 - 5,000 = 10,000. Pass.
    assert(vgprs.status[0] == COLOR_CYAN);
    
    // Lane 1: Balance 2,000 <= 10,000. No sweep. Skip (Black status).
    assert(vgprs.status[1] == COLOR_BLACK);
    
    // Lane 2: Unregistered account 99. Fail. Red status.
    assert(vgprs.status[2] == COLOR_RED);
    assert(huc.border_color == COLOR_RED);
    assert(huc.psg_frequency == WARNING_DRONE);
    assert(huc.blame_quarantine == true);

    // Final balance assert
    assert(registry[10].balance == 5000); // 15000 - 10000 sweep = 5000
    assert(registry[20].balance == 15000); // 5000 initial + 10000 sweep = 15000
    
    printf("   ✓ System registers and sweep validation outputs verified successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("HOGAN-HUDSON LEVEL 4 INTEGRATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
