#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "../inc/tsfi_frejlich_domain_governance.h"

int main(void) {
    printf("=== TESTING FREJLICH REALM DOMAIN GOVERNANCE SYSTEM IN PURE C ===\n\n");

    FrejlichRealmGovernanceState state;
    tsfi_frejlich_governance_init(&state);

    printf("1. Domain Sectors Registered:\n");
    for (uint32_t i = 0; i < state.sector_count; i++) {
        FrejlichDomainSector *s = &state.sectors[i];
        printf("   [%u] Sector: %-45s | Endowment: %10lu Saat | Emissaries: %u\n",
               s->sector_id, s->sector_name, (unsigned long)s->endowment_saat, s->active_emissaries);
    }

    printf("\n2. Logistics & Courier Conduits Established:\n");
    for (uint32_t i = 0; i < state.conduit_count; i++) {
        FrejlichLogisticsConduit *c = &state.conduits[i];
        printf("   [%u] Route: Sector %u -> Sector %u | Bandwidth: %6lu bps | Sec: %lu\n",
               c->route_id, c->origin_sector_id, c->dest_sector_id,
               (unsigned long)c->bandwidth_bps, (unsigned long)c->security_weight);
    }

    printf("\n3. Executing Sovereign Realm Reconciliation Pass...\n");
    char report[2048];
    bool ok = tsfi_frejlich_reconcile_realm(&state, 1786743100, report, sizeof(report));
    if (ok) {
        printf("\n%s\n", report);
    }

    return 0;
}
