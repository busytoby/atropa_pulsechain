#define _POSIX_C_SOURCE 200809L
#include "tsfi_lore_token_cache.h"
#include "tsfi_pulsechain_rpc.h"
#include "tsfi_tls.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_CACHE_RECORDS 256

static TsfiTokenRecordBin g_cache_table[MAX_CACHE_RECORDS];
static uint32_t g_cache_count = 0;
static bool g_cache_initialized = false;
static uint64_t g_network_query_counter = 0;

static void hex_to_bigint_raw(const char *hex, uint32_t *out_d) {
    memset(out_d, 0, 8 * sizeof(uint32_t));
    if (strncmp(hex, "0x", 2) == 0) hex += 2;
    int len = (int)strlen(hex);
    for (int i = 0; i < len; i++) {
        char c = hex[len - 1 - i];
        int val = (c >= '0' && c <= '9') ? (c - '0') :
                  (c >= 'a' && c <= 'f') ? (c - 'a' + 10) :
                  (c >= 'A' && c <= 'F') ? (c - 'A' + 10) : 0;
        int word = i / 8;
        int shift = (i % 8) * 4;
        if (word < 8) {
            out_d[word] |= ((uint32_t)val << shift);
        }
    }
}

bool tsfi_lore_cache_init(void) {
    if (g_cache_initialized) return true;
    memset(g_cache_table, 0, sizeof(g_cache_table));
    g_cache_count = 0;

    FILE *fp = fopen(TSFI_LORE_CACHE_PATH, "rb");
    if (!fp) {
        g_cache_initialized = true;
        return true;
    }

    TsfiTokenCacheHeader hdr;
    if (fread(&hdr, sizeof(hdr), 1, fp) != 1) {
        fclose(fp);
        g_cache_initialized = true;
        return true;
    }

    if (memcmp(hdr.magic, TSFI_LORE_CACHE_MAGIC, 8) != 0 || hdr.version != TSFI_LORE_CACHE_VERSION) {
        fclose(fp);
        g_cache_initialized = true;
        return true;
    }

    uint32_t count = hdr.record_count;
    if (count > MAX_CACHE_RECORDS) count = MAX_CACHE_RECORDS;

    size_t read_items = fread(g_cache_table, sizeof(TsfiTokenRecordBin), count, fp);
    g_cache_count = (uint32_t)read_items;
    fclose(fp);

    g_cache_initialized = true;
    return true;
}

bool tsfi_lore_cache_lookup(const char *token_address, TsfiTokenRecordBin *out_record) {
    if (!g_cache_initialized) {
        tsfi_lore_cache_init();
    }
    if (!token_address || !out_record) return false;

    // Fast in-memory lookup without console printing (sub-microsecond latency)
    for (uint32_t i = 0; i < g_cache_count; i++) {
        if (strcasecmp(g_cache_table[i].address, token_address) == 0) {
            memcpy(out_record, &g_cache_table[i], sizeof(TsfiTokenRecordBin));
            return true;
        }
    }
    return false;
}

bool tsfi_lore_cache_store(const TsfiTokenRecordBin *record) {
    if (!g_cache_initialized) {
        tsfi_lore_cache_init();
    }
    if (!record) return false;

    for (uint32_t i = 0; i < g_cache_count; i++) {
        if (strcasecmp(g_cache_table[i].address, record->address) == 0) {
            memcpy(&g_cache_table[i], record, sizeof(TsfiTokenRecordBin));
            return tsfi_lore_cache_flush();
        }
    }

    if (g_cache_count < MAX_CACHE_RECORDS) {
        memcpy(&g_cache_table[g_cache_count], record, sizeof(TsfiTokenRecordBin));
        g_cache_count++;
        return tsfi_lore_cache_flush();
    }

    return false;
}

bool tsfi_lore_cache_flush(void) {
    FILE *fp = fopen(TSFI_LORE_CACHE_PATH, "wb");
    if (!fp) return false;

    TsfiTokenCacheHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    memcpy(hdr.magic, TSFI_LORE_CACHE_MAGIC, 8);
    hdr.version = TSFI_LORE_CACHE_VERSION;
    hdr.record_count = g_cache_count;

    if (fwrite(&hdr, sizeof(hdr), 1, fp) != 1) {
        fclose(fp);
        return false;
    }

    if (g_cache_count > 0) {
        if (fwrite(g_cache_table, sizeof(TsfiTokenRecordBin), g_cache_count, fp) != g_cache_count) {
            fclose(fp);
            return false;
        }
    }

    fclose(fp);
    return true;
}

bool tsfi_lore_token_fetch_with_policy(const char *token_address, const char *treasury_wallet, bool force_refresh, TsfiTokenRecordBin *out_record) {
    if (!token_address || !out_record) return false;

    // 1. Check in-memory binary cache first unless force_refresh is explicitly requested
    if (!force_refresh) {
        if (tsfi_lore_cache_lookup(token_address, out_record)) {
            return true;
        }
    }

    // 2. Query via native clean-room C RPC (zero curl or external tools)
    char bal_call_data[256];
    const char *clean_wallet = (strncmp(treasury_wallet, "0x", 2) == 0) ? treasury_wallet + 2 : treasury_wallet;
    snprintf(bal_call_data, sizeof(bal_call_data), "0x70a08231000000000000000000000000%s", clean_wallet);

    char bal_hex[512] = {0};
    char supply_hex[512] = {0};
    char dec_hex[512] = {0};

    g_network_query_counter++;

    if (!tsfi_pulse_rpc_call(token_address, bal_call_data, bal_hex, sizeof(bal_hex))) {
        return false;
    }
    if (!tsfi_pulse_rpc_call(token_address, "0x18160ddd", supply_hex, sizeof(supply_hex))) {
        return false;
    }
    if (!tsfi_pulse_rpc_call(token_address, "0x313ce567", dec_hex, sizeof(dec_hex))) {
        return false;
    }

    TsfiTokenRecordBin rec;
    memset(&rec, 0, sizeof(rec));
    snprintf(rec.address, sizeof(rec.address), "%s", token_address);

    uint32_t bal_arr[8];
    uint32_t supply_arr[8];
    hex_to_bigint_raw(bal_hex, bal_arr);
    hex_to_bigint_raw(supply_hex, supply_arr);
    memcpy(rec.treasury_bal_d, bal_arr, sizeof(bal_arr));
    memcpy(rec.total_supply_d, supply_arr, sizeof(supply_arr));

    uint32_t dec_d[8];
    hex_to_bigint_raw(dec_hex, dec_d);
    rec.decimals = (uint8_t)(dec_d[0] == 0 ? 18 : dec_d[0]);
    rec.last_queried_timestamp = (uint64_t)time(NULL);

    // Store in binary cache immediately
    tsfi_lore_cache_store(&rec);

    memcpy(out_record, &rec, sizeof(TsfiTokenRecordBin));
    return true;
}

bool tsfi_lore_token_fetch_and_cache(const char *token_address, const char *treasury_wallet, TsfiTokenRecordBin *out_record) {
    return tsfi_lore_token_fetch_with_policy(token_address, treasury_wallet, false, out_record);
}

uint64_t tsfi_lore_cache_get_network_query_count(void) {
    return g_network_query_counter;
}

void tsfi_lore_cache_reset_query_counters(void) {
    g_network_query_counter = 0;
}

void tsfi_lore_cache_clear_in_memory(void) {
    memset(g_cache_table, 0, sizeof(g_cache_table));
    g_cache_count = 0;
    g_cache_initialized = true;
}
