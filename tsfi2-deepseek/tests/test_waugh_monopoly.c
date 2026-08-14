#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../inc/tsfi_waugh_monopoly.h"

int main(void) {
    printf("=== TESTING EVELYN WAUGH SOVEREIGN CURRENCY MONOPOLY IN PURE C ===\n\n");

    WaughMonetaryMonopolyState monopoly;
    tsfi_waugh_monopoly_init(&monopoly, 10000000); // 10M Saat vault reserve

    printf("1. Minting Sovereign Saat Bills under King Chen Jurchen's Seal:\n");
    // Mint 5 bills of 100,000 Saat denomination
    bool minted = tsfi_waugh_mint_sovereign_notes(&monopoly, 5, 100000, 2026);
    assert(minted);
    assert(monopoly.active_circulation_saat == 500000);

    for (uint32_t i = 0; i < monopoly.minted_notes_count; i++) {
        WaughSovereignNote *n = &monopoly.issued_notes[i];
        printf("   [Serial #%u] Denomination: %lu Saat | Monarch Seal: 0x%016lx\n",
               n->bill_serial, (unsigned long)n->denomination_saat, (unsigned long)n->monarch_seal_signature);
    }

    printf("\n2. Enforcing Currency Monopoly against Foreign Specie Inflow:\n");
    // Inflow of 1,000,000 foreign currency units at 1:1 parity (10000 bps)
    uint64_t saat_issued = 0;
    uint64_t seigniorage_fee = 0;
    bool converted = tsfi_waugh_enforce_currency_monopoly(&monopoly, 1000000, 10000, &saat_issued, &seigniorage_fee);
    assert(converted);
    printf("   Foreign Specie Converted: 1,000,000 units\n");
    printf("   Net Saat Issued:          %lu Saat\n", (unsigned long)saat_issued);
    printf("   Royal Seigniorage Tariff: %lu Saat (5%% Statutory Fee)\n", (unsigned long)seigniorage_fee);

    assert(saat_issued == 950000);
    assert(seigniorage_fee == 50000);

    printf("\n3. Executing Sovereign Currency Monopoly Audit...\n");
    char report[2048];
    uint64_t merkle = tsfi_waugh_monopoly_audit(&monopoly, report, sizeof(report));
    assert(merkle != 0);
    printf("\n%s\n", report);

    printf("=== ALL EVELYN WAUGH MONETARY MONOPOLY PROOFS PASSED DETERMINISTICALLY ===\n");
    return 0;
}
