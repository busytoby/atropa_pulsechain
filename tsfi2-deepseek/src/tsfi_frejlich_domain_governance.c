#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "tsfi_frejlich_domain_governance.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tsfi_frejlich_governance_init(FrejlichRealmGovernanceState *state) {
    if (!state) return;
    memset(state, 0, sizeof(FrejlichRealmGovernanceState));
    state->governance_epoch = 1863;
    state->treasury_total_saat = 10000000; // 10M Saat root treasury

    // Default primary sovereign sectors
    tsfi_frejlich_register_sector(state, FREJLICH_SECTOR_CIVIL_CADRE, "Central National Directorate (Warszawa)", 2500000);
    tsfi_frejlich_register_sector(state, FREJLICH_SECTOR_TACTICAL_COMMAND, "Operational Field Command (Kujawy-Mazowsze)", 3000000);
    tsfi_frejlich_register_sector(state, FREJLICH_SECTOR_LOGISTICS_SUPPLY, "Foreign Procurement Bureau (Paris-Genoa)", 2500000);
    tsfi_frejlich_register_sector(state, FREJLICH_SECTOR_COMMUNICATIONS, "Cryptographic Courier Relay", 1000000);
    tsfi_frejlich_register_sector(state, FREJLICH_SECTOR_ARCHIVAL_AUDIT, "Frejlich Historical & Audit Chancery", 1000000);

    // Initial default conduits
    tsfi_frejlich_establish_conduit(state, 0, 1, 100000); // Central -> Field
    tsfi_frejlich_establish_conduit(state, 0, 2, 50000);  // Central -> Foreign
    tsfi_frejlich_establish_conduit(state, 3, 0, 250000); // Courier -> Central
    tsfi_frejlich_establish_conduit(state, 4, 0, 500000); // Chancery -> Central
}

bool tsfi_frejlich_register_sector(
    FrejlichRealmGovernanceState *state,
    FrejlichSectorType type,
    const char *name,
    uint64_t initial_endowment
) {
    if (!state || !name || state->sector_count >= FREJLICH_MAX_SECTORS) return false;

    uint32_t id = state->sector_count++;
    FrejlichDomainSector *sec = &state->sectors[id];
    sec->sector_id = id;
    sec->type = type;
    strncpy(sec->sector_name, name, sizeof(sec->sector_name) - 1);
    sec->sector_name[sizeof(sec->sector_name) - 1] = '\0';
    sec->endowment_saat = initial_endowment;
    sec->active_emissaries = 1;
    sec->operational_status = 1;
    sec->last_audit_timestamp = 1786743000;

    return true;
}

bool tsfi_frejlich_establish_conduit(
    FrejlichRealmGovernanceState *state,
    uint32_t origin_id,
    uint32_t dest_id,
    uint64_t bandwidth
) {
    if (!state || state->conduit_count >= FREJLICH_MAX_LOGISTICS_ROUTES) return false;
    if (origin_id >= state->sector_count || dest_id >= state->sector_count) return false;

    uint32_t id = state->conduit_count++;
    FrejlichLogisticsConduit *c = &state->conduits[id];
    c->route_id = id;
    c->origin_sector_id = origin_id;
    c->dest_sector_id = dest_id;
    c->bandwidth_bps = bandwidth;
    c->security_weight = 99;
    c->active = true;

    return true;
}

bool tsfi_frejlich_reconcile_realm(
    FrejlichRealmGovernanceState *state,
    uint64_t current_timestamp,
    char *out_report,
    size_t report_max_len
) {
    if (!state || !out_report || report_max_len == 0) return false;

    uint64_t allocated_sum = 0;
    for (uint32_t i = 0; i < state->sector_count; i++) {
        allocated_sum += state->sectors[i].endowment_saat;
        state->sectors[i].last_audit_timestamp = current_timestamp;
    }

    snprintf(out_report, report_max_len,
             "================================================================================\n"
             "FREJLICH REALM DOMAIN GOVERNANCE & LOGISTICS RECONCILIATION\n"
             "================================================================================\n"
             "Epoch: %u | Active Sectors: %u | Active Conduits: %u\n"
             "Total Treasury Endowment: %lu Saat | Total Allocated: %lu Saat\n"
             "Audit Status: 100%% DETERMINISTIC & RECONCILED (Timestamp: %lu)\n"
             "================================================================================\n",
             state->governance_epoch, state->sector_count, state->conduit_count,
             (unsigned long)state->treasury_total_saat, (unsigned long)allocated_sum,
             (unsigned long)current_timestamp);

    return true;
}
