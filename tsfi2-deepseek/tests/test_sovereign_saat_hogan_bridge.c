#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../inc/tsfi_hogan.h"
#include "../inc/tsfi_frejlich_domain_governance.h"

int main(void) {
    printf("=== SOVEREIGN MONETARY POLICY & HOGAN BANK DDO TREASURY PROVER ===\n\n");

    // 1. Initialize Hogan Umbrella Banking System
    hogan_umbrella_system hogan_sys;
    tsfi_hogan_init(&hogan_sys);

    // 2. Initialize Frejlich Domain Governance
    FrejlichRealmGovernanceState realm_state;
    tsfi_frejlich_governance_init(&realm_state);

    printf("1. Provisioning Sovereign Sector Accounts in Hogan DDO System:\n");
    // Register 5 Domain Sectors as official Hogan Bank Corporate Accounts
    for (uint32_t i = 0; i < realm_state.sector_count; i++) {
        FrejlichDomainSector *sec = &realm_state.sectors[i];
        uint32_t acct_id = 9000 + sec->sector_id;
        int res = tsfi_hogan_register_account(&hogan_sys, acct_id, sec->endowment_saat);
        assert(res == 0);
        printf("   Account [%u] %-42s -> Balance: %10lu Saat\n",
               acct_id, sec->sector_name, (unsigned long)sec->endowment_saat);
    }

    printf("\n2. Executing Inter-Domain Monetary Dispatch (Central -> Field Command 500,000 Saat):\n");
    // Central Directorate (9000) dispatches 500,000 Saat to Field Command (9001) over ZMM Virtual Hardware
    int tx_res = tsfi_hogan_dispatch_tx(&hogan_sys, 9000, 9001, 500000, VM_ZMM);
    assert(tx_res == 0);
    printf("   Transaction dispatched over VM_ZMM pipeline cleanly.\n");

    printf("\n3. Running Statutory Overnight GL Reconciliation...\n");
    int rec_res = tsfi_hogan_overnight_reconciliation(&hogan_sys, "sovereign_hogan_lfs.dat.bin");
    assert(rec_res == 0);
    printf("   Overnight reconciliation complete. New Epoch: %u\n", hogan_sys.current_epoch);

    printf("\n4. Verifying Post-Settlement Ledger Balances:\n");
    for (uint32_t i = 0; i < realm_state.sector_count; i++) {
        uint32_t acct_id = 9000 + realm_state.sectors[i].sector_id;
        printf("   Account [%u] %-42s -> Balance: %10lu Saat\n",
               acct_id, realm_state.sectors[i].sector_name, (unsigned long)hogan_sys.accounts[i].balance);
    }

    assert(hogan_sys.accounts[0].balance == 2000000); // 2.5M - 0.5M = 2.0M
    assert(hogan_sys.accounts[1].balance == 3500000); // 3.0M + 0.5M = 3.5M

    printf("\n=== SOVEREIGN SAAT MONETARY POLICY FULLY RECONCILED WITH HOGAN BANK ===\n");
    return 0;
}
