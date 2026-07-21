#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_autodin_conway_tx.h"
#include "tsfi_lowpower_fet.h"

int tsfi_autodin_step_conway_tx_submitter(
    uint64_t start_tx_id,
    const char *submitter_addr,
    uint32_t total_escrow,
    uint32_t batch_limit_n,
    tsfi_autodin_conway_tx_summary_t *out_summary
) {
    if (!out_summary) return -1;
    memset(out_summary, 0, sizeof(tsfi_autodin_conway_tx_summary_t));

    out_summary->tx_sequence_id = start_tx_id;
    snprintf(out_summary->submitter_address, sizeof(out_summary->submitter_address),
             "%s", submitter_addr ? submitter_addr : "0x0000000000000000000000000000000000000000");
    out_summary->total_escrow_gas = total_escrow > 0 ? total_escrow : 10000;
    out_summary->batch_limit_n = batch_limit_n > 0 ? batch_limit_n : 64;

    // Deduct gas for this coroutine step (450 Gas) strictly from submitter's escrow pool
    out_summary->evm_gas_units = 450;
    out_summary->gas_consumed = out_summary->evm_gas_units;

    // Simulate processing ordered AUTODIN transactions up to batch limit N
    for (uint32_t i = 0; i < out_summary->batch_limit_n; i++) {
        out_summary->processed_count++;
    }

    out_summary->is_batch_complete = 1;
    // Calculate unspent gas refund for submitter
    out_summary->gas_refunded = (out_summary->total_escrow_gas > out_summary->gas_consumed) ?
                                (out_summary->total_escrow_gas - out_summary->gas_consumed) : 0;

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    out_summary->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(out_summary->tape_dat_bin, sizeof(out_summary->tape_dat_bin),
             "AUTODIN_SUBMITTER_GAS_%08X.DAT.BIN", (unsigned int)(start_tx_id & 0xFFFFFFFF));

    printf("[AUTODIN SUBMITTER GAS] Origin: %s | Escrow: %u G | Consumed: %u G | Refund: %u G | FET Power: %.4fW | File: %s\n",
           out_summary->submitter_address, out_summary->total_escrow_gas,
           out_summary->gas_consumed, out_summary->gas_refunded,
           out_summary->fet_power_watts, out_summary->tape_dat_bin);

    return 0; // Submitter funded coroutine step complete
}
