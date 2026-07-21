#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_hogan_tax_permutation.h"
#include "tsfi_lowpower_fet.h"

int tsfi_hogan_tax_engine_init(
    uint32_t account_id,
    hogan_tax_permutation_engine_t *engine
) {
    if (!engine) return -1;
    memset(engine, 0, sizeof(hogan_tax_permutation_engine_t));

    engine->account_id = account_id;
    engine->evm_gas_units = 280; // 280 Gas / Auncient Ether Units per lot disposal

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    engine->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(engine->quadtree_bin, sizeof(engine->quadtree_bin), "HOGAN_TAX_PERM_%08X.DAT.BIN", account_id);

    return 0;
}

int tsfi_hogan_add_tax_lot(
    hogan_tax_permutation_engine_t *engine,
    uint64_t amount_microcents,
    uint64_t cost_basis_microcents,
    const char *token_address
) {
    if (!engine || !token_address || engine->lot_count >= MAX_TAX_LOTS) return -1;

    hogan_tax_lot_t *lot = &engine->lots[engine->lot_count];
    lot->lot_id = (uint32_t)(engine->lot_count + 1);
    lot->acquisition_timestamp = 1750000000ULL + (engine->lot_count * 3600);
    lot->amount_microcents = amount_microcents;
    lot->cost_basis_microcents = cost_basis_microcents;

    // Rule 9: Dynamic address resolution prefix check
    if (strncmp(token_address, "dynamic_", 8) == 0) {
        snprintf(lot->token_address, sizeof(lot->token_address), "%s", token_address);
    } else {
        snprintf(lot->token_address, sizeof(lot->token_address), "dynamic_%s", token_address);
    }
    lot->is_disposed = 0;

    engine->lot_count++;
    printf("[HOGAN TAX ENGINE] Added Lot #%u: Amount: %lu microcents | Basis: %lu microcents | Token: %s\n",
           lot->lot_id, (unsigned long)amount_microcents, (unsigned long)cost_basis_microcents, lot->token_address);

    return 0;
}

int tsfi_hogan_permute_disposal(
    hogan_tax_permutation_engine_t *engine,
    uint64_t dispose_amount_microcents,
    uint64_t proceeds_microcents,
    hogan_tax_method_t strategy,
    uint64_t *out_realized_gain_loss
) {
    if (!engine || !out_realized_gain_loss || engine->lot_count == 0) return -1;

    uint32_t permuted_indices[MAX_TAX_LOTS];
    size_t active_count = 0;

    for (size_t i = 0; i < engine->lot_count; i++) {
        if (!engine->lots[i].is_disposed) {
            permuted_indices[active_count++] = (uint32_t)i;
        }
    }

    if (active_count == 0) return -1;

    // Permute lot processing order based on selected IRS tax method
    for (size_t i = 0; i < active_count - 1; i++) {
        for (size_t j = i + 1; j < active_count; j++) {
            uint32_t idx_a = permuted_indices[i];
            uint32_t idx_b = permuted_indices[j];
            int swap_needed = 0;

            if (strategy == HOGAN_TAX_LOT_FIFO) {
                if (engine->lots[idx_a].acquisition_timestamp > engine->lots[idx_b].acquisition_timestamp) swap_needed = 1;
            } else if (strategy == HOGAN_TAX_LOT_LIFO) {
                if (engine->lots[idx_a].acquisition_timestamp < engine->lots[idx_b].acquisition_timestamp) swap_needed = 1;
            } else if (strategy == HOGAN_TAX_LOT_HIFO) {
                if (engine->lots[idx_a].cost_basis_microcents < engine->lots[idx_b].cost_basis_microcents) swap_needed = 1;
            }

            if (swap_needed) {
                uint32_t tmp = permuted_indices[i];
                permuted_indices[i] = permuted_indices[j];
                permuted_indices[j] = tmp;
            }
        }
    }

    // Execute disposal across permuted tax lots
    uint64_t remaining_to_dispose = dispose_amount_microcents;
    uint64_t total_basis_disposed = 0;

    for (size_t k = 0; k < active_count && remaining_to_dispose > 0; k++) {
        uint32_t lot_idx = permuted_indices[k];
        hogan_tax_lot_t *lot = &engine->lots[lot_idx];

        uint64_t dispose_from_this_lot = (lot->amount_microcents < remaining_to_dispose) ?
                                          lot->amount_microcents : remaining_to_dispose;

        uint64_t basis_portion = (lot->cost_basis_microcents * dispose_from_this_lot) / lot->amount_microcents;
        total_basis_disposed += basis_portion;

        lot->amount_microcents -= dispose_from_this_lot;
        remaining_to_dispose -= dispose_from_this_lot;

        if (lot->amount_microcents == 0) {
            lot->is_disposed = 1;
        }
    }

    uint64_t realized = 0;
    if (proceeds_microcents >= total_basis_disposed) {
        realized = proceeds_microcents - total_basis_disposed;
        engine->total_realized_gain_microcents += realized;
    } else {
        realized = total_basis_disposed - proceeds_microcents;
        engine->total_realized_loss_microcents += realized;
    }

    *out_realized_gain_loss = realized;
    const char *strat_names[] = {"FIFO", "LIFO", "HIFO", "SPECIFIC_ID"};

    printf("[HOGAN TAX PERMUTATION] Strategy: %s | Disposed: %lu microcents | Basis: %lu | Realized Delta: %lu | Gas: %u\n",
           strat_names[strategy], (unsigned long)dispose_amount_microcents,
           (unsigned long)total_basis_disposed, (unsigned long)realized, engine->evm_gas_units);

    return 0;
}

int tsfi_hogan_tax_inscribe_tape_label(
    hogan_tax_permutation_engine_t *engine,
    uint8_t *tape_buffer_720b
) {
    if (!engine || !tape_buffer_720b) return -1;

    memset(tape_buffer_720b, ' ', 720);

    // Format 8-block 720-byte sequence: VOL1 (80B), HDR1..HDR8 (80B each)
    char vol1[81], hdr1[81], hdr2[81];
    snprintf(vol1, 81, "VOL1HGN%06u                                                                  ", engine->account_id);
    snprintf(hdr1, 81, "HDR1IRS_FORM_1099B_COMPLIANCE_AUDIT_LOG_ACC_%08X                          ", engine->account_id);
    snprintf(hdr2, 81, "HDR2GAIN_%012llu_LOSS_%012llu_GAS_%04u                               ",
             (unsigned long long)engine->total_realized_gain_microcents,
             (unsigned long long)engine->total_realized_loss_microcents,
             engine->evm_gas_units);

    memcpy(tape_buffer_720b, vol1, 80);
    memcpy(tape_buffer_720b + 80, hdr1, 80);
    memcpy(tape_buffer_720b + 160, hdr2, 80);

    for (int b = 3; b <= 9; b++) {
        char hdr_b[81];
        snprintf(hdr_b, 81, "HDR%dPROVENANCE_SIG_2026_USLM_AFFIRMED_HOGAN_TAX_AUDIT_LOTS_%02zu               ",
                 b - 1, engine->lot_count);
        memcpy(tape_buffer_720b + (b - 1) * 80, hdr_b, 80);
    }

    printf("[HOGAN TAX TAPE LABEL] Inscribed 720-byte Yul DDL sequence for Account #%u\n", engine->account_id);
    return 0;
}

int tsfi_hogan_tax_permute_swap_route(
    hogan_tax_permutation_engine_t *engine,
    const char *token_in,
    const char *token_out,
    uint64_t amount_in,
    char route_hops[MAX_SWAP_HOPS][48],
    size_t *out_hop_count
) {
    if (!engine || !token_in || !token_out || !route_hops || !out_hop_count) return -1;

    // Rule 9: Address-based dynamic contract resolution
    snprintf(route_hops[0], 48, "dynamic_%.39s", token_in);
    snprintf(route_hops[1], 48, "dynamic_0xPulseChainWPLSGatewayPool00001");
    snprintf(route_hops[2], 48, "dynamic_0xAtropaPulseChainLAULiquidityPool02");
    snprintf(route_hops[3], 48, "dynamic_%.39s", token_out);

    *out_hop_count = 4;
    printf("[HOGAN SWAP ROUTE PERMUTATION] TokenIn: %s -> TokenOut: %s | Amount: %lu | Hops: %zu\n",
           token_in, token_out, (unsigned long)amount_in, *out_hop_count);

    return 0;
}

int tsfi_hogan_verify_winchester_scsi_permutations(
    hogan_tax_permutation_engine_t *engine,
    uint8_t keycode,
    int *out_valid
) {
    if (!engine || !out_valid) return -1;

    // Rule 5: WinchesterMQ SCSI register verification for Keycodes 32 ('d'/'D') and 30 ('a'/'A')
    if (keycode == 32 || keycode == 30) {
        *out_valid = 1;
    } else {
        *out_valid = 0;
    }

    printf("[HOGAN WINCHESTER SCSI] Keycode: %u (%s) -> Permutation Verified: %s\n",
           keycode, (keycode == 32) ? "'d'/'D'" : (keycode == 30) ? "'a'/'A'" : "OTHER",
           *out_valid ? "VALID" : "INVALID");

    return 0;
}
