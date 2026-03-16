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

// Read a storage slot from an EVM contract
// out_hex_buffer should be at least 67 bytes ("0x" + 64 hex chars + null)
bool tsfi_pulse_rpc_get_storage_at(const char *address, const char *slot_hex, char *out_hex_buffer, size_t out_max_len);

#ifdef __cplusplus
}
#endif

#endif // TSFI_PULSECHAIN_RPC_H
