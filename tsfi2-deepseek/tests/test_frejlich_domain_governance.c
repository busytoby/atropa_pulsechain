#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../inc/tsfi_frejlich_domain_governance.h"

int main(void) {
    printf("=== EXTENDED INTEGRITY AUDIT: FREJLICH DOMAIN GOVERNANCE SYSTEM ===\n\n");

    FrejlichRealmGovernanceState state;
    tsfi_frejlich_governance_init(&state);

    printf("1. Initial Baseline Registration & Audit:\n");
    for (uint32_t i = 0; i < state.sector_count; i++) {
        FrejlichDomainSector *s = &state.sectors[i];
        printf("   Sector [%u] %-42s | DNA: 0x%016lx | Endowment: %lu Saat\n",
               s->sector_id, s->sector_name, (unsigned long)s->dna_hash, (unsigned long)s->endowment_saat);
    }

    uint64_t initial_merkle = tsfi_frejlich_compute_merkle_root(&state);
    printf("\n   -> Computed Root Merkle: 0x%016lx\n", (unsigned long)initial_merkle);
    assert(initial_merkle != 0);

    printf("\n2. Dynamic Sector Expansion (Registering Technical Innovation Sector)...\n");
    bool added = tsfi_frejlich_register_sector(&state, FREJLICH_SECTOR_CIVIL_CADRE, "Auncient Wavelet Research Lab", 500000);
    assert(added);
    printf("   Sector [%u] %s registered successfully.\n", state.sectors[state.sector_count - 1].sector_id,
           state.sectors[state.sector_count - 1].sector_name);

    uint64_t updated_merkle = tsfi_frejlich_compute_merkle_root(&state);
    printf("   -> Updated Root Merkle:  0x%016lx\n", (unsigned long)updated_merkle);
    assert(updated_merkle != initial_merkle);

    printf("\n3. Logistics Conduit Extension & High-Bandwidth Linking...\n");
    bool conduit_added = tsfi_frejlich_establish_conduit(&state, 0, state.sector_count - 1, 1000000);
    assert(conduit_added);
    printf("   Conduit established between Sector 0 and Sector %u (1,000,000 bps).\n", state.sector_count - 1);

    printf("\n4. Full Sovereign Realm Cryptographic Reconciliation:\n");
    char report[2048];
    bool ok = tsfi_frejlich_reconcile_realm(&state, 1786743200, report, sizeof(report));
    assert(ok);
    printf("\n%s\n", report);

    printf("=== ALL FREJLICH GOVERNANCE PROOFS PASSED DETERMINISTICALLY ===\n");
    return 0;
}
