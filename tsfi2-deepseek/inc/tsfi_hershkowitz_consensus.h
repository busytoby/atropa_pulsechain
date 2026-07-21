#ifndef TSFI_HERSHKOWITZ_CONSENSUS_H
#define TSFI_HERSHKOWITZ_CONSENSUS_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint64_t block_height;         // Block height evaluated for consensus
    uint32_t transaction_count;    // Total transactions in proposed block
    double chi_square_val;         // Hershkowitz Chi-Square stat
    uint8_t statistical_valid;     // 1 = Pass statistical test, 0 = Reject
    uint8_t quorum_confirmed;      // 1 = Pass matrix transshipment quorum, 0 = Reject
    uint32_t evm_gas_units;        // 410 Gas / Auncient Ether Units
    double fet_power_watts;        // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];        // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_hershkowitz_consensus_t;

/* Initialize a Hershkowitz Transactional Consensus Validator */
int tsfi_hershkowitz_consensus_init(
    uint64_t block_height,
    tsfi_hershkowitz_consensus_t *cons
);

/* Evaluate statistical and matrix transshipment consensus on a proposed block ledger */
int tsfi_hershkowitz_consensus_validate(
    tsfi_hershkowitz_consensus_t *cons,
    const uint32_t *tx_distribution,
    size_t dist_count,
    uint32_t node_quorum_mask
);

#endif // TSFI_HERSHKOWITZ_CONSENSUS_H
