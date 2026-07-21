#ifndef TSFI_AUTODIN_ZMM_TX_H
#define TSFI_AUTODIN_ZMM_TX_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char from_addr[43];
    char to_addr[43];        // dynamic_<address>
    uint64_t chain_id;       // Replay protection (e.g. 369)
    uint32_t nonce;          // Sender register nonce
    uint32_t selector;       // Yul contract function selector
    uint8_t calldata[256];
    size_t calldata_len;
    uint32_t max_fee_per_fet;     // EIP-1559 Equivalent
    uint32_t max_priority_fee;   // Priority Tip
    uint8_t process_clearance;    // Security Governance tag
    char sig_key[32];            // SIG_2026_USLM_AFFIRMED
} tsfi_autodin_zmm_tx_req_t;

typedef struct {
    char tx_hash_hex[65];
    char block_hash_hex[65];
    uint64_t block_number;
    uint32_t cumulative_fet_used;
    uint32_t status; // 1 = Success, 0 = Reverted
    int is_mined;
} tsfi_autodin_zmm_tx_receipt_t;

/* Block-secure ZMM VM transaction dispatcher performing eth_sendTransaction duties over AUTODIN */
int autodin_send_zmm_tx(
    const tsfi_autodin_zmm_tx_req_t *tx_req,
    tsfi_autodin_zmm_tx_receipt_t *out_receipt
);

/* Query ZMM VM account/register nonce */
uint32_t autodin_get_zmm_nonce(const char *account_addr);

/* Estimate ZMM VM execution cost based on FET discharge depletion physics */
uint32_t autodin_estimate_fet_cost(uint32_t selector, size_t calldata_len);

#endif // TSFI_AUTODIN_ZMM_TX_H
