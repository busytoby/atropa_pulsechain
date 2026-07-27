#ifndef TSFI_NSA_SIMONSPECK_H
#define TSFI_NSA_SIMONSPECK_H

#include <stdint.h>
#include <stddef.h>

// SIMON 64/128 implementation
// 64-bit block size (two 32-bit words), 128-bit key size (four 32-bit words)
#define SIMON_64_128_ROUNDS 44

typedef struct {
    uint32_t round_keys[SIMON_64_128_ROUNDS];
} tsfi_simon_key;

void tsfi_simon_64_128_expand_key(const uint8_t *key_bytes, tsfi_simon_key *key_out);
void tsfi_simon_64_128_encrypt(const uint8_t *plain, const tsfi_simon_key *key, uint8_t *cipher_out);
void tsfi_simon_64_128_decrypt(const uint8_t *cipher, const tsfi_simon_key *key, uint8_t *plain_out);

// SPECK 64/128 implementation
// 64-bit block size (two 32-bit words), 128-bit key size (four 32-bit words)
#define SPECK_64_128_ROUNDS 27

typedef struct {
    uint32_t round_keys[SPECK_64_128_ROUNDS];
} tsfi_speck_key;

void tsfi_speck_64_128_expand_key(const uint8_t *key_bytes, tsfi_speck_key *key_out);
void tsfi_speck_64_128_encrypt(const uint8_t *plain, const tsfi_speck_key *key, uint8_t *cipher_out);
void tsfi_speck_64_128_decrypt(const uint8_t *cipher, const tsfi_speck_key *key, uint8_t *plain_out);

// Integration helpers for RAMAC sector / WinchesterMQ protection
int tsfi_nsa_encrypt_ramac_sector(uint32_t cylinder, uint32_t head, uint32_t sector, uint8_t *data, size_t len, const uint8_t *key_bytes, int use_speck);
int tsfi_nsa_decrypt_ramac_sector(uint32_t cylinder, uint32_t head, uint32_t sector, uint8_t *data, size_t len, const uint8_t *key_bytes, int use_speck);

#endif // TSFI_NSA_SIMONSPECK_H
