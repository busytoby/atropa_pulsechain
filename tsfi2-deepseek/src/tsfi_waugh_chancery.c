#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "tsfi_waugh_chancery.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FNV1A_64_OFFSET 0xcbf29ce484222325ULL
#define FNV1A_64_PRIME  0x100000001b3ULL

static uint64_t fnv1a_hash(const void *data, size_t len) {
    const uint8_t *ptr = (const uint8_t*)data;
    uint64_t hash = FNV1A_64_OFFSET;
    for (size_t i = 0; i < len; i++) {
        hash ^= ptr[i];
        hash *= FNV1A_64_PRIME;
    }
    return hash;
}

void tsfi_waugh_chancery_init(WaughChanceryState *state) {
    if (!state) return;
    memset(state, 0, sizeof(WaughChanceryState));
    state->total_customs_revenue_saat = 0;

    // Initial default diplomatic dispatches (Scoop / Black Mischief satire themes)
    tsfi_waugh_dispatch_cable(state, WAUGH_DISPATCH_IMPERIAL_DECREE, "Monarch Palace", "All Provinces", "Decree of Universal Mathematical Modernization & Saat Primacy", 2026);
    tsfi_waugh_dispatch_cable(state, WAUGH_DISPATCH_PRESS_CABLE, "The Daily Beast (Foreign Bureau)", "London", "Lord Copper Informs: Sovereign Realm Balance Completely Sound", 2026);
    
    // Initial foreign legation treaty
    tsfi_waugh_ratify_treaty(state, "Legation of Azania & Foreign Consortiums", 850, 250000);
}

bool tsfi_waugh_dispatch_cable(
    WaughChanceryState *state,
    WaughDispatchType type,
    const char *origin,
    const char *destination,
    const char *headline,
    uint64_t epoch
) {
    if (!state || !origin || !destination || !headline || state->cable_count >= WAUGH_MAX_CABLES) return false;

    uint32_t idx = state->cable_count++;
    WaughDiplomaticCable *c = &state->cables[idx];
    c->cable_id = 5000 + idx;
    c->type = type;
    strncpy(c->origin, origin, sizeof(c->origin) - 1);
    strncpy(c->destination, destination, sizeof(c->destination) - 1);
    strncpy(c->content_headline, headline, sizeof(c->content_headline) - 1);
    c->transmission_epoch = epoch;
    c->verified_by_censor = true;

    return true;
}

bool tsfi_waugh_ratify_treaty(
    WaughChanceryState *state,
    const char *party_name,
    uint64_t tariff_bps,
    uint64_t tribute_saat
) {
    if (!state || !party_name || state->treaty_count >= WAUGH_MAX_TREATIES) return false;

    uint32_t idx = state->treaty_count++;
    WaughTreatyProtocol *t = &state->treaties[idx];
    t->treaty_id = 8000 + idx;
    strncpy(t->party_name, party_name, sizeof(t->party_name) - 1);
    t->customs_tariff_rate_bps = tariff_bps;
    t->sovereign_recognition_saat = tribute_saat;
    t->ratified = true;

    state->total_customs_revenue_saat += tribute_saat;
    return true;
}

uint64_t tsfi_waugh_chancery_audit(
    WaughChanceryState *state,
    char *out_report,
    size_t max_len
) {
    if (!state || !out_report || max_len == 0) return 0;

    uint64_t combined = FNV1A_64_OFFSET;
    for (uint32_t i = 0; i < state->cable_count; i++) {
        WaughDiplomaticCable *c = &state->cables[i];
        uint64_t chash = fnv1a_hash(c->content_headline, strlen(c->content_headline)) ^ c->cable_id;
        combined ^= chash;
        combined *= FNV1A_64_PRIME;
    }
    for (uint32_t i = 0; i < state->treaty_count; i++) {
        WaughTreatyProtocol *t = &state->treaties[i];
        uint64_t thash = fnv1a_hash(t->party_name, strlen(t->party_name)) ^ t->sovereign_recognition_saat;
        combined ^= thash;
        combined *= FNV1A_64_PRIME;
    }
    combined ^= state->total_customs_revenue_saat;
    state->chancery_merkle_proof = combined;

    snprintf(out_report, max_len,
             "================================================================================\n"
             "EVELYN WAUGH DIPLOMATIC CHANCERY & FOREIGN PROTOCOL AUDIT\n"
             "================================================================================\n"
             "Sovereign Monarch: King Chen Jurchen\n"
             "Active Diplomatic Cables: %u | Ratified Foreign Treaties: %u\n"
             "Customs & Tribute Vault:   %10lu Saat\n"
             "Chancery Merkle Proof:     0x%016lx\n"
             "Status: 100%% RATIFIED & DIPLOMATICALLY RECONCILED\n"
             "================================================================================\n",
             state->cable_count, state->treaty_count,
             (unsigned long)state->total_customs_revenue_saat,
             (unsigned long)state->chancery_merkle_proof);

    return state->chancery_merkle_proof;
}
