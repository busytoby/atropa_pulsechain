#ifndef TSFI_LORE_TOKEN_CACHE_H
#define TSFI_LORE_TOKEN_CACHE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TSFI_LORE_CACHE_MAGIC "TSFI_DAT"
#define TSFI_LORE_CACHE_VERSION 1
#define TSFI_LORE_CACHE_PATH "assets/treasury_tokens_cache.dat.bin"

typedef struct __attribute__((packed)) {
    char address[44];
    char symbol[32];
    char name[64];
    uint8_t decimals;
    uint8_t padding[3];
    uint32_t total_supply_d[8];
    uint32_t treasury_bal_d[8];
    uint64_t last_queried_timestamp;
    uint32_t block_number;
    uint32_t holders_count;
} TsfiTokenRecordBin;

typedef struct __attribute__((packed)) {
    char magic[8];
    uint32_t version;
    uint32_t record_count;
    uint8_t reserved[16];
} TsfiTokenCacheHeader;

// Load binary cache from disk into memory
bool tsfi_lore_cache_init(void);

// Fast in-memory lookup (< 1000 ns latency)
bool tsfi_lore_cache_lookup(const char *token_address, TsfiTokenRecordBin *out_record);

// Update or insert a token record into memory and flush to .dat.bin
bool tsfi_lore_cache_store(const TsfiTokenRecordBin *record);

// Query on-chain state via pure clean-room C RPC (no external curl/exec), populating cache
bool tsfi_lore_token_fetch_and_cache(const char *token_address, const char *treasury_wallet, TsfiTokenRecordBin *out_record);

// Flush memory table to .dat.bin file
bool tsfi_lore_cache_flush(void);

#ifdef __cplusplus
}
#endif

#endif // TSFI_LORE_TOKEN_CACHE_H
