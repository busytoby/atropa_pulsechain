#include "tsfi_pulsechain_rpc.h"
#include "tsfi_tls.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

typedef struct {
    const char *address;
    const char *symbol;
    const char *name;
    int decimals;
} HardcodedToken;

static const HardcodedToken g_known_tokens[] = {
    { "0xa1077a294dde1b09bb078844df40758a5d0f9a27", "WPLS", "Wrapped Pulse", 18 },
    { "0x959c5ad5c5ad5c5ad5c5ad5c5ad5c5ad5c5ad5cd", "PLSX", "PulseX", 18 },
    { "0x15d38573d2feeb82e7ad5187ab8c1d52810b1f07", "USDC-Eth", "USD Coin (from Ethereum)", 6 },
    { "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48", "USDC", "USD Coin", 6 },
    { "0xefd766ccb8c15e5e9f813af7b2809857baa53a1f", "DAI-Eth", "Dai Stablecoin (from Ethereum)", 18 },
    { "0x6b175474e89094c44da98b954eedeac495271d0f", "DAI", "Dai Stablecoin", 18 },
    { "0x0cb81b54a05e0547d2d08c4a9e273a7d4c72b9eb", "USDT-Eth", "Tether USD (from Ethereum)", 6 },
    { "0xdac17f958d2ee523a2206206994597c13d831ec7", "USDT", "Tether USD", 6 },
    { "0xcc78a0acdf847a2c1714d2a925bb4477df5d48a6", "Atropa", "Atropa", 18 },
    { "0xd6c31ba0754c4383a41c0e9df042c62b5e918f6d", "TeddyBear", "TeddyBear", 18 }
};

#define KNOWN_TOKENS_COUNT (sizeof(g_known_tokens)/sizeof(g_known_tokens[0]))

static uint64_t parse_hex_val(const char *hex, size_t len) {
    char temp[65] = {0};
    if (len > 64) len = 64;
    for (size_t i = 0; i < len; i++) {
        temp[i] = hex[i];
    }
    temp[len] = '\0';
    return strtoull(temp, NULL, 16);
}

static double parse_hex_double(const char *hex, size_t len) {
    double val = 0.0;
    for (size_t i = 0; i < len; i++) {
        char c = hex[i];
        int digit = 0;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
        else break;
        val = val * 16.0 + digit;
    }
    return val;
}

static void decode_abi_string(const char *hex, char *out, size_t max) {
    if (!hex || strlen(hex) < 130) {
        strncpy(out, "UNKNOWN", max);
        return;
    }
    const char *ptr = hex;
    if (ptr[0] == '0' && (ptr[1] == 'x' || ptr[1] == 'X')) ptr += 2;
    uint64_t len = parse_hex_val(ptr + 64, 64);
    if (len > max - 1) len = max - 1;
    const char *chars_hex = ptr + 128;
    for (uint64_t i = 0; i < len; i++) {
        out[i] = (char)parse_hex_val(chars_hex + i * 2, 2);
    }
    out[len] = '\0';
}

static uint64_t decode_abi_uint(const char *hex) {
    if (!hex) return 18;
    const char *ptr = hex;
    if (ptr[0] == '0' && (ptr[1] == 'x' || ptr[1] == 'X')) ptr += 2;
    size_t len = strlen(ptr);
    if (len > 64) ptr += (len - 64);
    return strtoull(ptr, NULL, 16);
}

static void resolve_token(const char *addr, char *out_symbol, char *out_name, uint64_t *out_decimals) {
    // 1. Check hardcoded list first (case-insensitive)
    for (size_t i = 0; i < KNOWN_TOKENS_COUNT; i++) {
        if (strcasecmp(addr, g_known_tokens[i].address) == 0) {
            strcpy(out_symbol, g_known_tokens[i].symbol);
            strcpy(out_name, g_known_tokens[i].name);
            *out_decimals = g_known_tokens[i].decimals;
            return;
        }
    }

    // 2. Fallback to RPC query
    char call_buf[1024];
    strcpy(out_symbol, "UNKNOWN");
    strcpy(out_name, "Unknown Token");
    *out_decimals = 18;

    if (tsfi_pulse_rpc_call(addr, "0x95d89b41", call_buf, sizeof(call_buf))) {
        decode_abi_string(call_buf, out_symbol, 64);
    }
    if (tsfi_pulse_rpc_call(addr, "0x06fdde03", call_buf, sizeof(call_buf))) {
        decode_abi_string(call_buf, out_name, 128);
    }
    if (tsfi_pulse_rpc_call(addr, "0x313ce567", call_buf, sizeof(call_buf))) {
        *out_decimals = decode_abi_uint(call_buf);
    }
}

int main(void) {
    printf("=== Auncient PulseChain Swap Scanner (C-Native HTTPS) ===\n");
    
    char block_buf[128] = {0};
    const char *block_payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_blockNumber\",\"params\":[],\"id\":1}";
    
    if (!tsfi_pulse_rpc_exec_raw(block_payload, block_buf, sizeof(block_buf))) {
        printf("Error: Failed to fetch latest block height from mainnet.\n");
        return 1;
    }
    
    uint64_t current_block = strtoull(block_buf, NULL, 16);
    // Query 5 blocks behind tip to ensure logs index is fully indexed
    uint64_t query_block = current_block - 5;
    printf("Latest Block: %lu, Querying Block: %lu (0x%lx)\n", current_block, query_block, query_block);
    
    char logs_payload[512];
    snprintf(logs_payload, sizeof(logs_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getLogs\",\"params\":[{\"fromBlock\":\"0x%lx\",\"toBlock\":\"0x%lx\",\"topics\":[\"0xd78ad95fa46c994b6551d0da85fc275fe613ce37657fb8d5e3d130840159d822\"]}],\"id\":1}",
             query_block, query_block);
             
    static char logs_buf[262144] = {0};
    if (!tsfi_pulse_rpc_exec_raw(logs_payload, logs_buf, sizeof(logs_buf))) {
        printf("Error: Failed to fetch logs for block %lu\n", query_block);
        return 1;
    }
    
    double total_price = 0.0;
    int count = 0;
    
    const char *ptr = logs_buf;
    while ((ptr = strstr(ptr, "\"address\":\""))) {
        ptr += 11;
        char pool_address[64] = {0};
        const char *end_addr = strchr(ptr, '"');
        if (end_addr) {
            strncpy(pool_address, ptr, end_addr - ptr);
        }
        
        const char *data_ptr = strstr(ptr, "\"data\":\"");
        if (data_ptr) {
            data_ptr += 8;
            if (strncmp(data_ptr, "0x", 2) == 0) data_ptr += 2;
            
            double amt0_in = parse_hex_double(data_ptr, 64);
            double amt1_in = parse_hex_double(data_ptr + 64, 64);
            double amt0_out = parse_hex_double(data_ptr + 128, 64);
            double amt1_out = parse_hex_double(data_ptr + 192, 64);
            
            char t0_addr[64] = {0};
            char t1_addr[64] = {0};
            char call_buf[1024];
            
            // Get token0 address
            if (tsfi_pulse_rpc_call(pool_address, "0x0dfe1681", call_buf, sizeof(call_buf))) {
                if (strlen(call_buf) >= 40) {
                    snprintf(t0_addr, sizeof(t0_addr), "0x%s", call_buf + strlen(call_buf) - 40);
                }
            }
            
            // Get token1 address (with fallback to WPLS if token1 reverts or fails)
            if (tsfi_pulse_rpc_call(pool_address, "0xd21225a3", call_buf, sizeof(call_buf))) {
                if (strlen(call_buf) >= 40) {
                    snprintf(t1_addr, sizeof(t1_addr), "0x%s", call_buf + strlen(call_buf) - 40);
                }
            } else {
                // Fallback: if token0 is not WPLS, assume token1 is WPLS. Otherwise assume PLSX.
                if (strcasecmp(t0_addr, "0xa1077a294dde1b09bb078844df40758a5d0f9a27") != 0) {
                    strcpy(t1_addr, "0xa1077a294dde1b09bb078844df40758a5d0f9a27"); // WPLS
                } else {
                    strcpy(t1_addr, "0x959c5ad5c5ad5c5ad5c5ad5c5ad5c5ad5c5ad5cd"); // PLSX
                }
            }
            
            if (strlen(t0_addr) > 0 && strlen(t1_addr) > 0) {
                char t0_sym[64] = {0}, t0_name[128] = {0};
                char t1_sym[64] = {0}, t1_name[128] = {0};
                uint64_t t0_dec = 18, t1_dec = 18;
                
                resolve_token(t0_addr, t0_sym, t0_name, &t0_dec);
                resolve_token(t1_addr, t1_sym, t1_name, &t1_dec);
                
                double val0_in = amt0_in / pow(10, t0_dec);
                double val1_in = amt1_in / pow(10, t1_dec);
                double val0_out = amt0_out / pow(10, t0_dec);
                double val1_out = amt1_out / pow(10, t1_dec);
                
                double from_val = 0.0, to_val = 0.0;
                const char *from_sym = "", *to_sym = "";
                const char *from_name = "", *to_name = "";
                double price = 0.0;
                
                if (amt0_in > 0.0) {
                    from_sym = t0_sym; from_name = t0_name; from_val = val0_in;
                    to_sym = t1_sym; to_name = t1_name; to_val = val1_out;
                    if (val0_in > 0.0) price = val1_out / val0_in;
                } else {
                    from_sym = t1_sym; from_name = t1_name; from_val = val1_in;
                    to_sym = t0_sym; to_name = t0_name; to_val = val0_out;
                    if (val1_in > 0.0) price = val0_out / val1_in;
                }
                
                printf("Swap %d (Pool %s):\n", count + 1, pool_address);
                printf("  From: %.4f %s (%s)\n", from_val, from_sym, from_name);
                printf("  To:   %.4f %s (%s)\n", to_val, to_sym, to_name);
                printf("  Price: %.6f\n", price);
                
                total_price += price;
                count++;
            }
        }
        ptr = data_ptr;
        if (count >= 5) break;
    }
    
    if (count > 0) {
        printf("\nAverage Swap Price: %.6f (Scanned %d swaps)\n", total_price / count, count);
    } else {
        printf("No valid Swap V2 logs parsed for block %lu.\n", query_block);
    }
    
    return 0;
}
