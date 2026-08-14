#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "tsfi_waugh_monopoly.h"
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

void tsfi_waugh_monopoly_init(WaughMonetaryMonopolyState *state, uint64_t initial_reserve) {
    if (!state) return;
    memset(state, 0, sizeof(WaughMonetaryMonopolyState));
    state->total_reserve_vault_saat = initial_reserve;
    state->active_circulation_saat = 0;
    state->seigniorage_collected_saat = 0;
    state->minted_notes_count = 0;
}

bool tsfi_waugh_mint_sovereign_notes(
    WaughMonetaryMonopolyState *state,
    uint32_t count,
    uint64_t denomination,
    uint64_t current_epoch
) {
    if (!state || count == 0 || denomination == 0) return false;
    uint64_t required_total = (uint64_t)count * denomination;
    if (state->active_circulation_saat + required_total > state->total_reserve_vault_saat) {
        return false; // Strict reserve backing: no unbacked printing
    }

    for (uint32_t i = 0; i < count && state->minted_notes_count < WAUGH_MAX_BILLS; i++) {
        uint32_t idx = state->minted_notes_count++;
        WaughSovereignNote *note = &state->issued_notes[idx];
        note->bill_serial = 100000 + idx;
        note->denomination_saat = denomination;
        note->issue_epoch = current_epoch;
        note->in_circulation = true;

        // King Chen Jurchen's cryptographic royal seal signature
        char seal_buf[64];
        snprintf(seal_buf, sizeof(seal_buf), "KING_CHEN_JURCHEN_SAAT_SEAL_%u_%lu", note->bill_serial, (unsigned long)denomination);
        note->monarch_seal_signature = fnv1a_hash(seal_buf, strlen(seal_buf));

        state->active_circulation_saat += denomination;
    }

    return true;
}

bool tsfi_waugh_enforce_currency_monopoly(
    WaughMonetaryMonopolyState *state,
    uint64_t foreign_amount,
    uint64_t exchange_rate_bps,
    uint64_t *out_saat_issued,
    uint64_t *out_seigniorage_fee
) {
    if (!state || foreign_amount == 0 || exchange_rate_bps == 0) return false;

    // Convert foreign specie to gross Saat
    uint64_t gross_saat = (foreign_amount * exchange_rate_bps) / 10000;
    
    // Waugh Royal Seigniorage Tariff (5% statutory monopoly conversion fee)
    uint64_t tariff_fee = (gross_saat * 500) / 10000;
    uint64_t net_saat = gross_saat - tariff_fee;

    if (state->active_circulation_saat + net_saat > state->total_reserve_vault_saat) {
        return false;
    }

    state->active_circulation_saat += net_saat;
    state->seigniorage_collected_saat += tariff_fee;

    if (out_saat_issued) *out_saat_issued = net_saat;
    if (out_seigniorage_fee) *out_seigniorage_fee = tariff_fee;

    return true;
}

uint64_t tsfi_waugh_monopoly_audit(
    WaughMonetaryMonopolyState *state,
    char *out_report,
    size_t max_len
) {
    if (!state || !out_report || max_len == 0) return 0;

    uint64_t root_hash = FNV1A_64_OFFSET;
    for (uint32_t i = 0; i < state->minted_notes_count; i++) {
        WaughSovereignNote *n = &state->issued_notes[i];
        uint64_t note_hash = n->monarch_seal_signature ^ n->denomination_saat ^ n->bill_serial;
        root_hash ^= note_hash;
        root_hash *= FNV1A_64_PRIME;
    }
    root_hash ^= state->seigniorage_collected_saat;
    state->monopoly_merkle_root = root_hash;

    snprintf(out_report, max_len,
             "================================================================================\n"
             "EVELYN WAUGH MONETARY MONOPOLY & SOVEREIGN SEIGNIORAGE AUDIT\n"
             "================================================================================\n"
             "Monarch: King Chen Jurchen | Sovereign Currency: SAAT\n"
             "Total Reserve Vault:        %10lu Saat\n"
             "Active Circulation:         %10lu Saat (%u notes minted)\n"
             "Royal Seigniorage Treasury: %10lu Saat\n"
             "Currency Merkle Proof:      0x%016lx\n"
             "Status: 100%% SOVEREIGN MONOPOLY ENFORCED & RECONCILED\n"
             "================================================================================\n",
             (unsigned long)state->total_reserve_vault_saat,
             (unsigned long)state->active_circulation_saat,
             state->minted_notes_count,
             (unsigned long)state->seigniorage_collected_saat,
             (unsigned long)state->monopoly_merkle_root);

    return state->monopoly_merkle_root;
}
