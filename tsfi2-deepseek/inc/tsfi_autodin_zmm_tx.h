#ifndef TSFI_AUTODIN_ZMM_TX_H
#define TSFI_AUTODIN_ZMM_TX_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char tx_hash_hex[65];
    char contract_address[43]; // dynamic_<address>
    uint32_t nonce;
    uint32_t fet_cost;
    int is_mined;
} tsfi_autodin_zmm_tx_receipt_t;

/* Dispatch a ZMM VM transaction over AUTODIN message channels (ZMM equivalent of eth_sendTransaction) */
int autodin_send_zmm_tx(
    const char *contract_addr,
    uint32_t selector,
    const uint8_t *calldata,
    size_t calldata_len,
    uint8_t process_clearance,
    tsfi_autodin_zmm_tx_receipt_t *out_receipt
);

/* Query ZMM VM account/register nonce */
uint32_t autodin_get_zmm_nonce(const char *contract_addr);

/* Estimate ZMM VM execution cost based on FET discharge depletion physics */
uint32_t autodin_estimate_fet_cost(uint32_t selector, size_t calldata_len);

#endif // TSFI_AUTODIN_ZMM_TX_H
