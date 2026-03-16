#ifndef TSFI_PULSECHAIN_H
#define TSFI_PULSECHAIN_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Represents a 256-bit hash or key, aligned to 64 bytes for AVX-512 boundaries if needed later
typedef struct {
    uint8_t data[32];
} __attribute__((aligned(64))) TsfiPulseHash;

// PulseChain/Ethereum Address (20 bytes of actual data, padded for alignment)
typedef struct {
    uint8_t address[20];
} __attribute__((aligned(32))) TsfiPulseAddress;

// PulseChain SECP256k1 Public Key (64 bytes raw uncompressed without 0x04 prefix)
typedef struct {
    uint8_t pubkey[64];
} __attribute__((aligned(64))) TsfiPulsePublicKey;

// PulseChain SECP256k1 Private Key (32 bytes)
typedef struct {
    uint8_t privkey[32];
} __attribute__((aligned(64))) TsfiPulsePrivateKey;

// Compute Keccak-256 standard hash (0x01 padding rule, not SHA3 0x06 rule)
// Ensures alignment with EVM and PulseChain RPC standards.
void tsfi_pulse_keccak256(const uint8_t *in, size_t in_len, TsfiPulseHash *out);

// Convert a raw 20-byte address into an EIP-55 checksummed hex string
// `out_hex` must be at least 43 bytes: "0x" + 40 chars + null terminator
void tsfi_pulse_eip55_format(const TsfiPulseAddress *addr, char *out_hex);

// Parse a hexadecimal string into a 20-byte address, validating EIP-55 if mixed case
// Returns true if parsing/checksum succeeds, false if invalid.
bool tsfi_pulse_parse_address(const char *hex_in, TsfiPulseAddress *out_addr);

// Derive a PulseChain/Ethereum Address from a Raw 64-byte Uncompressed Public Key.
// Per Ethereum standard: address = keccak256(pubkey)[12..31]
void tsfi_pulse_pubkey_to_address(const TsfiPulsePublicKey *pub, TsfiPulseAddress *out_addr);

#ifdef __cplusplus
}
#endif

#endif // TSFI_PULSECHAIN_H