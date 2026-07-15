#ifndef TSFI_PULSECHAIN_RPC_H
#define TSFI_PULSECHAIN_RPC_H

#include "tsfi_pulsechain.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Fetch data from an EVM contract using an eth_call
// out_hex_buffer must be large enough to hold the result (e.g. 1024 bytes)
bool tsfi_pulse_rpc_call(const char *to_address, const char *data_hex, char *out_hex_buffer, size_t out_max_len);

// Fetch data from an EVM contract using an eth_call with a specific from address
bool tsfi_pulse_rpc_call_from(const char *to_address, const char *from_address, const char *data_hex, char *out_hex_buffer, size_t out_max_len);

// Read a storage slot from an EVM contract
// out_hex_buffer should be at least 67 bytes ("0x" + 64 hex chars + null)
bool tsfi_pulse_rpc_get_storage_at(const char *address, const char *slot_hex, char *out_hex_buffer, size_t out_max_len);

// Send a signed raw transaction to the network
// out_tx_hash should be at least 67 bytes to hold the resulting transaction hash
bool tsfi_pulse_rpc_send_raw_transaction(const char *signed_tx_hex, char *out_tx_hash, size_t out_max_len);

// Send an unsigned transaction to WinchesterMQ (target) directly
bool tsfi_pulse_rpc_send_wmq_transaction(const char *to_address, const char *data_hex);

// Publish a formatted cross-system input command message to WinchesterMQ
void tsfi_thunk_publish_mq(const char *cmd);

// Execute a raw JSON-RPC call to the local EVM/Anvil node
bool tsfi_pulse_rpc_exec_raw(const char *json_payload, char *out_buffer, size_t out_max_len);

// Execute a raw JSON-RPC call and return the raw unparsed JSON response body
bool tsfi_pulse_rpc_exec_raw_body(const char *json_payload, char *out_buffer, size_t out_max_len);

// Fetch token holders list from the PulseScan BlockScout API
bool tsfi_pulse_explorer_get_holders(const char *token_addr, char *out_buffer, size_t out_max_len);

#ifdef __cplusplus
}
#endif

#endif // TSFI_PULSECHAIN_RPC_H
