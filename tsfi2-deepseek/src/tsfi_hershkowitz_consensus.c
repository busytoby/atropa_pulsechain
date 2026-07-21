#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_hershkowitz_consensus.h"
#include "tsfi_lowpower_fet.h"

int tsfi_hershkowitz_consensus_init(
    uint64_t block_height,
    tsfi_hershkowitz_consensus_t *cons
) {
    if (!cons) return -1;
    memset(cons, 0, sizeof(tsfi_hershkowitz_consensus_t));

    cons->block_height = block_height;
    cons->evm_gas_units = 410; // 410 Gas / Auncient Ether Units

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    cons->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(cons->tape_dat_bin, sizeof(cons->tape_dat_bin), "HERSHKOWITZ_CONSENSUS_%08llX.DAT.BIN", (unsigned long long)block_height);
    return 0;
}

int tsfi_hershkowitz_consensus_validate(
    tsfi_hershkowitz_consensus_t *cons,
    const uint32_t *tx_distribution,
    size_t dist_count,
    uint32_t node_quorum_mask
) {
    if (!cons || !tx_distribution || dist_count == 0) return -1;

    double chi_sq = 0.0;
    uint32_t total_tx = 0;
    double expected_per_bin = 10.0; // Benchmark expected Poisson rate

    for (size_t i = 0; i < dist_count; i++) {
        total_tx += tx_distribution[i];
        double diff = (double)tx_distribution[i] - expected_per_bin;
        chi_sq += (diff * diff) / expected_per_bin;
    }

    cons->transaction_count = total_tx;
    cons->chi_square_val = chi_sq;

    // Chi-square critical threshold validation (< 16.919 for alpha=0.05)
    cons->statistical_valid = (chi_sq <= 16.919) ? 1 : 0;

    // Matrix transshipment node quorum threshold validation (Minimum 3 nodes in bitmask)
    uint32_t active_nodes = 0;
    for (int b = 0; b < 32; b++) {
        if ((node_quorum_mask >> b) & 1) active_nodes++;
    }
    cons->quorum_confirmed = (active_nodes >= 3) ? 1 : 0;

    uint8_t final_consensus = (cons->statistical_valid && cons->quorum_confirmed) ? 1 : 0;

    printf("[HERSHKOWITZ CONSENSUS] Block: %llu | Total Tx: %u | Chi-Sq: %.4f | Stat: %s | Quorum: %s (%u nodes) | FINAL CONSENSUS: %s | Gas: %u | File: %s\n",
           (unsigned long long)cons->block_height, cons->transaction_count, cons->chi_square_val,
           cons->statistical_valid ? "VALID" : "REJECTED",
           cons->quorum_confirmed ? "CONFIRMED" : "INSUFFICIENT", active_nodes,
           final_consensus ? "CONFIRMED & INSCRIBED" : "REJECTED",
           cons->evm_gas_units, cons->tape_dat_bin);

    return final_consensus ? 0 : 1; // 0 = Consensus Achieved, 1 = Consensus Failed
}
