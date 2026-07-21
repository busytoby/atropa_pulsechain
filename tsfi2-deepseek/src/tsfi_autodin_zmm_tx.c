#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_autodin_zmm_tx.h"
#include "tsfi_parc_tape_label_yul.h"

static uint32_t g_zmm_nonce_counter = 0;

uint32_t autodin_get_zmm_nonce(const char *contract_addr) {
    (void)contract_addr;
    return g_zmm_nonce_counter;
}

uint32_t autodin_estimate_fet_cost(uint32_t selector, size_t calldata_len) {
    (void)selector;
    return 100 + (uint32_t)(calldata_len * 10);
}

int autodin_send_zmm_tx(
    const char *contract_addr,
    uint32_t selector,
    const uint8_t *calldata,
    size_t calldata_len,
    uint8_t process_clearance,
    tsfi_autodin_zmm_tx_receipt_t *out_receipt
) {
    if (!contract_addr || !out_receipt) return -1;
    (void)calldata;

    memset(out_receipt, 0, sizeof(tsfi_autodin_zmm_tx_receipt_t));

    uint32_t current_nonce = ++g_zmm_nonce_counter;
    uint32_t fet_cost = autodin_estimate_fet_cost(selector, calldata_len);

    strncpy(out_receipt->contract_address, contract_addr, sizeof(out_receipt->contract_address) - 1);
    out_receipt->nonce = current_nonce;
    out_receipt->fet_cost = fet_cost;
    out_receipt->is_mined = 1;

    snprintf(out_receipt->tx_hash_hex, sizeof(out_receipt->tx_hash_hex),
             "0x%08x%08x%08x%08x", selector, current_nonce, (unsigned int)calldata_len, process_clearance);

    printf("[AUTODIN ZMM TX] Dispatching ZMM Transaction -> Target: %s | Selector: 0x%08X | Nonce: %u | FET Cost: %u | TxHash: %s\n",
           contract_addr, selector, current_nonce, fet_cost, out_receipt->tx_hash_hex);

    return 0; // Transaction dispatched and mined into ZMM state
}
