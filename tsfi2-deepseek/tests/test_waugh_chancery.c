#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../inc/tsfi_waugh_chancery.h"

int main(void) {
    printf("=== TESTING EVELYN WAUGH DIPLOMATIC CHANCERY SYSTEM IN PURE C ===\n\n");

    WaughChanceryState chancery;
    tsfi_waugh_chancery_init(&chancery);

    printf("1. Diplomatic & Press Cable Register:\n");
    for (uint32_t i = 0; i < chancery.cable_count; i++) {
        WaughDiplomaticCable *c = &chancery.cables[i];
        printf("   [Cable #%u] %-25s -> %-15s | %s\n",
               c->cable_id, c->origin, c->destination, c->content_headline);
    }

    printf("\n2. Ratified Foreign Legation Treaties:\n");
    for (uint32_t i = 0; i < chancery.treaty_count; i++) {
        WaughTreatyProtocol *t = &chancery.treaties[i];
        printf("   [Treaty #%u] %-40s | Tariff: %4lu bps | Recognition: %lu Saat\n",
               t->treaty_id, t->party_name, (unsigned long)t->customs_tariff_rate_bps,
               (unsigned long)t->sovereign_recognition_saat);
    }

    printf("\n3. Dispatching New Press Cable from Special Correspondent (William Boot)...\n");
    bool c_ok = tsfi_waugh_dispatch_cable(&chancery, WAUGH_DISPATCH_PRESS_CABLE, "Ishmaelia Desk", "The Daily Beast",
                                         "UPHEAVAL AVERTED: King Chen Jurchen's Mathematical Machine Reigns Supreme", 2026);
    assert(c_ok);

    printf("4. Ratifying New Maritime Trade Protocol with Neutral Consortiums...\n");
    bool t_ok = tsfi_waugh_ratify_treaty(&chancery, "Neutral Maritime Trade Guilds", 450, 150000);
    assert(t_ok);

    printf("\n5. Executing Chancery Cryptographic Audit...\n");
    char report[2048];
    uint64_t proof = tsfi_waugh_chancery_audit(&chancery, report, sizeof(report));
    assert(proof != 0);
    printf("\n%s\n", report);

    assert(chancery.total_customs_revenue_saat == 400000); // 250,000 + 150,000
    printf("=== ALL EVELYN WAUGH DIPLOMATIC CHANCERY PROOFS PASSED DETERMINISTICALLY ===\n");
    return 0;
}
