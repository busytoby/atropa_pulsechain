#include "lau_memory.h"
#include "tsfi_block_monitor.h"
#include "tsfi_pulsechain_rpc.h"
#include "tsfi_raw.h"
#include "tsfi_io.h"
#include "tsfi_qing_graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>
#include <dirent.h>
#define MAX_PENDING_BLOCKS 100
#define KNOWN_TOKENS_COUNT (sizeof(g_known_tokens)/sizeof(g_known_tokens[0]))
#define MAX_SWAP_EDGES 1024
#define MAX_DISCOVERED_TOKENS 512
#define g_discovered_tokens (g_mcp_rdbms_table->rows)
#define g_discovered_tokens_count (g_mcp_rdbms_table->count)
#define MAX_CONTRACT_CACHE_ROWS 1024
#define MAX_POOL_CACHE 256
#define MAX_BLOCK_HISTORY 50


const HardcodedToken g_known_tokens[10] = {
    { "0xa1077a294dde1b09bb078844df40758a5d0f9a27", "WPLS", "Wrapped Pulse", 18 },
    { "0x959c5ad5c5ad5c5ad5c5ad5c5ad5c5ad5c5ad5cd", "PLSX", "PulseX", 18 },
    { "0x15d38573d2feeb82e7ad5187ab8c1d52810b1f07", "USDC-Eth", "USD Coin (from Ethereum)", 6 },
    { "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48", "USDC", "USD Coin", 6 },
    { "0xefd766ccb38eaf1dfd701853bfce31359239f305", "DAI-Eth", "Dai Stablecoin (from Ethereum)", 18 },
    { "0x6b175474e89094c44da98b954eedeac495271d0f", "DAI", "Dai Stablecoin", 18 },
    { "0x0cb81b54a05e0547d2d08c4a9e273a7d4c72b9eb", "USDT-Eth", "Tether USD (from Ethereum)", 6 },
    { "0xdac17f958d2ee523a2206206994597c13d831ec7", "USDT", "Tether USD", 6 },
    { "0xcc78a0acdf847a2c1714d2a925bb4477df5d48a6", "Atropa", "Atropa", 18 },
    { "0xd6c31ba0754c4383a41c0e9df042c62b5e918f6d", "TeddyBear", "TeddyBear", 18 }
};

static void decode_abi_string(const char *hex, char *out, size_t max) {
    if (!hex || strlen(hex) < 130) {
        strncpy(out, "UNKNOWN", max);
        return;
    }
    const char *ptr = hex;
    if (ptr[0] == '0' && (ptr[1] == 'x' || ptr[1] == 'X')) ptr += 2;
    
    // Parse length of dynamic string
    char len_hex[65] = {0};
    strncpy(len_hex, ptr + 64, 64);
    uint64_t len = strtoull(len_hex, NULL, 16);
    
    if (len > max - 1) len = max - 1;
    const char *chars_hex = ptr + 128;
    for (uint64_t i = 0; i < len; i++) {
        char temp[3] = { chars_hex[i * 2], chars_hex[i * 2 + 1], '\0' };
        out[i] = (char)strtoull(temp, NULL, 16);
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

static void decode_abi_address(const char *hex, char *out_addr) {
    if (!hex) {
        out_addr[0] = '\0';
        return;
    }
    const char *ptr = hex;
    if (ptr[0] == '0' && (ptr[1] == 'x' || ptr[1] == 'X')) ptr += 2;
    size_t len = strlen(ptr);
    if (len >= 64) {
        // ABI addresses are right-aligned 20-byte values in a 32-byte word (64 hex characters)
        // So the address occupies the last 40 hex characters.
        strcpy(out_addr, "0x");
        strncat(out_addr, ptr + 24, 40);
    } else if (len == 40) {
        strcpy(out_addr, "0x");
        strcat(out_addr, ptr);
    } else {
        out_addr[0] = '\0';
    }
}

LauRdbmsTable *g_mcp_rdbms_table = NULL;
void ensure_rdbms_table_initialized(void) {
    if (!g_mcp_rdbms_table) {
        g_mcp_rdbms_table = (LauRdbmsTable*)lau_malloc_wired(sizeof(LauRdbmsTable));
        if (g_mcp_rdbms_table) {
            memset(g_mcp_rdbms_table, 0, sizeof(LauRdbmsTable));
            g_mcp_rdbms_table->active = 1;
            g_mcp_rdbms_table->col_count = 6;
            g_mcp_rdbms_table->capacity = MAX_DISCOVERED_TOKENS;
        }
    }
}

void save_discovered_tokens(void) {
    ensure_rdbms_table_initialized();
    if (!g_mcp_rdbms_table) return;
    FILE *f = fopen("assets/contract_metadata.dat.bin", "wb");
    if (!f) {
        mkdir("assets", 0755);
        f = fopen("assets/contract_metadata.dat.bin", "wb");
    }
    if (f) {
        fwrite(g_mcp_rdbms_table, sizeof(LauRdbmsTable), 1, f);
        fclose(f);
    }
}

void load_discovered_tokens(void) {
    ensure_rdbms_table_initialized();
    if (!g_mcp_rdbms_table) return;
    FILE *f = fopen("assets/contract_metadata.dat.bin", "rb");
    if (f) {
        LauRdbmsTable temp_table;
        size_t read_blocks = fread(&temp_table, sizeof(LauRdbmsTable), 1, f);
        if (read_blocks == 1) {
            g_mcp_rdbms_table->active = temp_table.active;
            g_mcp_rdbms_table->col_count = temp_table.col_count;
            g_mcp_rdbms_table->count = temp_table.count;
            g_mcp_rdbms_table->capacity = temp_table.capacity;
            memcpy(g_mcp_rdbms_table->rows, temp_table.rows, sizeof(temp_table.rows));
            fprintf(stderr, "[LOAD_DB] Successfully loaded %d tokens from assets/contract_metadata.dat.bin\n", g_mcp_rdbms_table->count);
        } else {
            fprintf(stderr, "[LOAD_DB] Failed to read from assets/contract_metadata.dat.bin (read %zu blocks)\n", read_blocks);
        }
        fclose(f);
    } else {
        fprintf(stderr, "[LOAD_DB] No assets/contract_metadata.dat.bin found, starting fresh\n");
    }
}

LauContractCacheTable *g_contract_cache_table = NULL;
void ensure_contract_cache_initialized(void) {
    if (!g_contract_cache_table) {
        g_contract_cache_table = (LauContractCacheTable*)lau_malloc_wired(sizeof(LauContractCacheTable));
        if (g_contract_cache_table) {
            memset(g_contract_cache_table, 0, sizeof(LauContractCacheTable));
            g_contract_cache_table->active = 1;
        }
    }
}

ContractCacheBstNode *g_cache_bst_root = NULL;
static void free_cache_bst(ContractCacheBstNode *node) {
    if (!node) return;
    free_cache_bst(node->left);
    free_cache_bst(node->right);
    free(node);
}

static ContractCacheBstNode* insert_cache_bst(ContractCacheBstNode *node, uint64_t key, uint32_t index) {
    if (!node) {
        ContractCacheBstNode *new_node = malloc(sizeof(ContractCacheBstNode));
        if (new_node) {
            new_node->key = key;
            new_node->index = index;
            new_node->left = NULL;
            new_node->right = NULL;
        }
        return new_node;
    }
    if (key < node->key) {
        node->left = insert_cache_bst(node->left, key, index);
    } else if (key > node->key) {
        node->right = insert_cache_bst(node->right, key, index);
    } else {
        node->index = index;
    }
    return node;
}

static uint64_t addr_to_key(const char *addr) {
    const char *p = addr;
    if (strncmp(p, "0x", 2) == 0) p += 2;
    size_t len = strlen(p);
    if (len > 16) {
        p += (len - 16);
    }
    return strtoull(p, NULL, 16);
}

static void rebuild_cache_bst(void) {
    free_cache_bst(g_cache_bst_root);
    g_cache_bst_root = NULL;
    if (!g_contract_cache_table) return;
    for (uint32_t i = 0; i < g_contract_cache_table->count; i++) {
        uint64_t key = addr_to_key(g_contract_cache_table->rows[i].address);
        g_cache_bst_root = insert_cache_bst(g_cache_bst_root, key, i);
    }
}

void save_contract_cache_table(void) {
    ensure_contract_cache_initialized();
    if (!g_contract_cache_table) return;
    FILE *f = fopen("assets/contract_cache.dat.bin", "wb");
    if (f) {
        fwrite(g_contract_cache_table, sizeof(LauContractCacheTable), 1, f);
        fclose(f);
    }
}

void load_contract_cache_table(void) {
    ensure_contract_cache_initialized();
    if (!g_contract_cache_table) return;
    FILE *f = fopen("assets/contract_cache.dat.bin", "rb");
    if (f) {
        LauContractCacheTable temp_table;
        size_t read_blocks = fread(&temp_table, sizeof(LauContractCacheTable), 1, f);
        if (read_blocks == 1) {
            g_contract_cache_table->active = temp_table.active;
            g_contract_cache_table->count = temp_table.count;
            memcpy(g_contract_cache_table->rows, temp_table.rows, sizeof(temp_table.rows));
            fprintf(stderr, "[LOAD_DB] Successfully loaded %d contract cache rows from assets/contract_cache.dat.bin\n", g_contract_cache_table->count);
            rebuild_cache_bst();
        }
        fclose(f);
    }
}

int get_contract_cache_status(const char *addr, bool *is_contract, bool *is_lp, char *t0, char *t0_sym, char *t1, char *t1_sym) {
    ensure_contract_cache_initialized();
    if (!g_contract_cache_table) return 0;
    
    uint64_t key = addr_to_key(addr);
    ContractCacheBstNode *curr = g_cache_bst_root;
    while (curr) {
        if (key < curr->key) {
            curr = curr->left;
        } else if (key > curr->key) {
            curr = curr->right;
        } else {
            uint32_t idx = curr->index;
            if (strcasecmp(g_contract_cache_table->rows[idx].address, addr) == 0) {
                *is_contract = (g_contract_cache_table->rows[idx].is_contract == 2);
                *is_lp = (g_contract_cache_table->rows[idx].is_lp == 2);
                if (t0) strcpy(t0, g_contract_cache_table->rows[idx].token0);
                if (t0_sym) strcpy(t0_sym, g_contract_cache_table->rows[idx].token0_sym);
                if (t1) strcpy(t1, g_contract_cache_table->rows[idx].token1);
                if (t1_sym) strcpy(t1_sym, g_contract_cache_table->rows[idx].token1_sym);
                return 1;
            }
            break;
        }
    }
    
    for (uint32_t i = 0; i < g_contract_cache_table->count; i++) {
        if (strcasecmp(g_contract_cache_table->rows[i].address, addr) == 0) {
            *is_contract = (g_contract_cache_table->rows[i].is_contract == 2);
            *is_lp = (g_contract_cache_table->rows[i].is_lp == 2);
            if (t0) strcpy(t0, g_contract_cache_table->rows[i].token0);
            if (t0_sym) strcpy(t0_sym, g_contract_cache_table->rows[i].token0_sym);
            if (t1) strcpy(t1, g_contract_cache_table->rows[i].token1);
            if (t1_sym) strcpy(t1_sym, g_contract_cache_table->rows[i].token1_sym);
            return 1;
        }
    }
    return 0;
}

void set_contract_cache_status(const char *addr, bool is_contract, bool is_lp, const char *t0, const char *t0_sym, const char *t1, const char *t1_sym) {
    ensure_contract_cache_initialized();
    if (!g_contract_cache_table) return;
    for (uint32_t i = 0; i < g_contract_cache_table->count; i++) {
        if (strcasecmp(g_contract_cache_table->rows[i].address, addr) == 0) {
            g_contract_cache_table->rows[i].is_contract = is_contract ? 2 : 1;
            g_contract_cache_table->rows[i].is_lp = is_lp ? 2 : 1;
            if (t0) strcpy(g_contract_cache_table->rows[i].token0, t0);
            if (t0_sym) strcpy(g_contract_cache_table->rows[i].token0_sym, t0_sym);
            if (t1) strcpy(g_contract_cache_table->rows[i].token1, t1);
            if (t1_sym) strcpy(g_contract_cache_table->rows[i].token1_sym, t1_sym);
            save_contract_cache_table();
            return;
        }
    }
    if (g_contract_cache_table->count < MAX_CONTRACT_CACHE_ROWS) {
        uint32_t idx = g_contract_cache_table->count++;
        strcpy(g_contract_cache_table->rows[idx].address, addr);
        g_contract_cache_table->rows[idx].is_contract = is_contract ? 2 : 1;
        g_contract_cache_table->rows[idx].is_lp = is_lp ? 2 : 1;
        if (t0) strcpy(g_contract_cache_table->rows[idx].token0, t0);
        else g_contract_cache_table->rows[idx].token0[0] = '\0';
        if (t0_sym) strcpy(g_contract_cache_table->rows[idx].token0_sym, t0_sym);
        else g_contract_cache_table->rows[idx].token0_sym[0] = '\0';
        if (t1) strcpy(g_contract_cache_table->rows[idx].token1, t1);
        else g_contract_cache_table->rows[idx].token1[0] = '\0';
        if (t1_sym) strcpy(g_contract_cache_table->rows[idx].token1_sym, t1_sym);
        else g_contract_cache_table->rows[idx].token1_sym[0] = '\0';
        uint64_t key = addr_to_key(addr);
        g_cache_bst_root = insert_cache_bst(g_cache_bst_root, key, idx);
        save_contract_cache_table();
    }
}

static bool is_fallback_symbol(const char *sym);
void add_discovered_token(const char *addr, const char *symbol, const char *name, uint64_t decimals) {
    ensure_rdbms_table_initialized();
    if (!g_mcp_rdbms_table) return;
    for (uint32_t i = 0; i < g_mcp_rdbms_table->count; i++) {
        if (strcasecmp(g_mcp_rdbms_table->rows[i].address, addr) == 0) {
            if (is_fallback_symbol(g_mcp_rdbms_table->rows[i].symbol)) {
                if (symbol && !is_fallback_symbol(symbol)) {
                    printf("[DEBUG_ADD_TOKEN] Overwriting fallback %s -> %s for %s\n", g_mcp_rdbms_table->rows[i].symbol, symbol, addr);
                    strcpy(g_mcp_rdbms_table->rows[i].symbol, symbol);
                    strcpy(g_mcp_rdbms_table->rows[i].name, name);
                    g_mcp_rdbms_table->rows[i].decimals = decimals;
                    save_discovered_tokens();
                }
            }
            return;
        }
    }
    if (g_mcp_rdbms_table->count < MAX_DISCOVERED_TOKENS) {
        uint32_t idx = g_mcp_rdbms_table->count++;
        strcpy(g_mcp_rdbms_table->rows[idx].address, addr);
        strcpy(g_mcp_rdbms_table->rows[idx].symbol, symbol);
        strcpy(g_mcp_rdbms_table->rows[idx].name, name);
        g_mcp_rdbms_table->rows[idx].decimals = decimals;
        g_mcp_rdbms_table->rows[idx].price_pls = 0.0;
        save_discovered_tokens();
    }
}

void check_and_register_rpc_token_metadata(const char *to_addr, const char *data_hex, const char *response_hex) {
    const char *data_ptr = data_hex;
    if (strncmp(data_ptr, "0x", 2) == 0) data_ptr += 2;
    
    const char *resp_ptr = response_hex;
    if (strncmp(resp_ptr, "0x", 2) == 0) resp_ptr += 2;
    if (strlen(resp_ptr) == 0) return;
    
    if (strcasecmp(data_ptr, "313ce567") == 0) { // decimals
        uint64_t decimals = decode_abi_uint(response_hex);
        ensure_rdbms_table_initialized();
        if (g_mcp_rdbms_table) {
            for (uint32_t i = 0; i < g_mcp_rdbms_table->count; i++) {
                if (strcasecmp(g_mcp_rdbms_table->rows[i].address, to_addr) == 0) {
                    g_mcp_rdbms_table->rows[i].decimals = decimals;
                    save_discovered_tokens();
                    break;
                }
            }
        }
    } else if (strcasecmp(data_ptr, "95d89b41") == 0) { // symbol
        char symbol[64] = {0};
        decode_abi_string(response_hex, symbol, sizeof(symbol));
        if (strlen(symbol) > 0 && !is_fallback_symbol(symbol)) {
            ensure_rdbms_table_initialized();
            if (g_mcp_rdbms_table) {
                bool found = false;
                for (uint32_t i = 0; i < g_mcp_rdbms_table->count; i++) {
                    if (strcasecmp(g_mcp_rdbms_table->rows[i].address, to_addr) == 0) {
                        strcpy(g_mcp_rdbms_table->rows[i].symbol, symbol);
                        save_discovered_tokens();
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    add_discovered_token(to_addr, symbol, symbol, 18);
                }
            }
        }
    } else if (strcasecmp(data_ptr, "06fdde03") == 0) { // name
        char name[128] = {0};
        decode_abi_string(response_hex, name, sizeof(name));
        if (strlen(name) > 0) {
            ensure_rdbms_table_initialized();
            if (g_mcp_rdbms_table) {
                for (uint32_t i = 0; i < g_mcp_rdbms_table->count; i++) {
                    if (strcasecmp(g_mcp_rdbms_table->rows[i].address, to_addr) == 0) {
                        strcpy(g_mcp_rdbms_table->rows[i].name, name);
                        save_discovered_tokens();
                        break;
                    }
                }
            }
        }
    }
}

static bool is_fallback_symbol(const char *sym) {
    if (!sym || strlen(sym) == 0) return true;
    if (strcmp(sym, "UNKNOWN") == 0 || strcmp(sym, "UNKN") == 0 || strcmp(sym, "UNKNOWN_TOKEN") == 0) return true;
    size_t len = strlen(sym);
    if (len >= 8 && strcmp(sym + len - 8, "Contract") == 0) return true;
    if (len >= 6 && strcmp(sym + len - 6, "Router") == 0) return true;
    return false;
}

void resolve_token(const char *addr, char *out_symbol, char *out_name, uint64_t *out_decimals) {
    ensure_rdbms_table_initialized();
    char fallback_symbol[128] = "UNKNOWN";
    char fallback_name[128] = "Unknown Token";
    bool has_fallback = false;

    if (g_mcp_rdbms_table) {
        for (uint32_t i = 0; i < g_mcp_rdbms_table->count; i++) {
            if (strcasecmp(g_mcp_rdbms_table->rows[i].address, addr) == 0) {
                if (!is_fallback_symbol(g_mcp_rdbms_table->rows[i].symbol)) {
                    strcpy(out_symbol, g_mcp_rdbms_table->rows[i].symbol);
                    strcpy(out_name, g_mcp_rdbms_table->rows[i].name);
                    *out_decimals = g_mcp_rdbms_table->rows[i].decimals;
                    return;
                } else {
                    strcpy(fallback_symbol, g_mcp_rdbms_table->rows[i].symbol);
                    strcpy(fallback_name, g_mcp_rdbms_table->rows[i].name);
                    *out_decimals = g_mcp_rdbms_table->rows[i].decimals;
                    has_fallback = true;
                }
            }
        }
    }

    for (size_t i = 0; i < KNOWN_TOKENS_COUNT; i++) {
        if (strcasecmp(addr, g_known_tokens[i].address) == 0) {
            strcpy(out_symbol, g_known_tokens[i].symbol);
            strcpy(out_name, g_known_tokens[i].name);
            *out_decimals = g_known_tokens[i].decimals;
            add_discovered_token(addr, out_symbol, out_name, *out_decimals);
            return;
        }
    }

    // Try querying the chain to see if it supports ERC20 name/symbol
    char chain_symbol[64] = {0};
    char chain_name[128] = {0};
    uint64_t chain_decimals = 18;
    bool success = false;

    char call_buf[1024];
    if (tsfi_pulse_rpc_call(addr, "0x95d89b41", call_buf, sizeof(call_buf))) {
        decode_abi_string(call_buf, chain_symbol, 64);
        if (strlen(chain_symbol) > 0 && !is_fallback_symbol(chain_symbol)) {
            success = true;
        }
    }
    if (tsfi_pulse_rpc_call(addr, "0x06fdde03", call_buf, sizeof(call_buf))) {
        decode_abi_string(call_buf, chain_name, 128);
    }
    if (tsfi_pulse_rpc_call(addr, "0x313ce567", call_buf, sizeof(call_buf))) {
        chain_decimals = decode_abi_uint(call_buf);
    }

    if (success && strlen(chain_symbol) > 0) {
        strcpy(out_symbol, chain_symbol);
        if (strlen(chain_name) > 0) {
            strcpy(out_name, chain_name);
        } else {
            strcpy(out_name, chain_symbol);
        }
        *out_decimals = chain_decimals;
    } else {
        // Fallback to seeded variable names
        if (has_fallback && strlen(fallback_symbol) > 0) {
            strcpy(out_symbol, fallback_symbol);
            strcpy(out_name, fallback_name);
        } else {
            strcpy(out_symbol, "UNKNOWN");
            strcpy(out_name, "Unknown Token");
            *out_decimals = 18;
        }
    }

    add_discovered_token(addr, out_symbol, out_name, *out_decimals);
}

void seed_tokens_from_addresses_sol(void) {
    FILE *f = fopen("solidity/addresses.sol", "r");
    if (!f) {
        f = fopen("../solidity/addresses.sol", "r");
    }
    if (!f) {
        printf("[SEED] Failed to open addresses.sol from any location!\n");
    }
    if (f) {
        printf("[SEED] Successfully opened addresses.sol\n");
        int count_parsed = 0;
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            // Skip commented out lines or blank/empty lines
            char *trim_line = line;
            while (*trim_line == ' ' || *trim_line == '\t') trim_line++;
            if (strncmp(trim_line, "//", 2) == 0 || strncmp(trim_line, "/*", 2) == 0) {
                continue;
            }

            char *p = strstr(line, "address(0x");
            if (p) {
                p += 8;
                char addr[64] = {0};
                char *end = strchr(p, ')');
                if (end && (end - p) < 50) {
                    strncpy(addr, p, end - p);
                    char symbol[128] = "UNKNOWN";
                    char name[128] = "UNKNOWN";
                    uint64_t decimals = 18;
                    
                    char var_name[128] = {0};
                    char *const_ptr = strstr(line, "constant ");
                    if (const_ptr) {
                        const_ptr += 9;
                        char *eq_ptr = strchr(const_ptr, '=');
                        if (eq_ptr) {
                            char *end_var = eq_ptr - 1;
                            while (end_var > const_ptr && (*end_var == ' ' || *end_var == '\t')) end_var--;
                            size_t var_len = end_var - const_ptr + 1;
                            if (var_len < 120) {
                                strncpy(var_name, const_ptr, var_len);
                                var_name[var_len] = '\0';
                            }
                        }
                    }
                    if (strlen(var_name) > 0) {
                        char comment_symbol[128] = {0};
                        char *comment_ptr = strstr(line, "//");
                        if (comment_ptr && comment_ptr > p) {
                            comment_ptr += 2;
                            while (*comment_ptr == ' ' || *comment_ptr == '\t') {
                                comment_ptr++;
                            }
                            char *end_comment = comment_ptr + strlen(comment_ptr) - 1;
                            while (end_comment >= comment_ptr && (*end_comment == ' ' || *end_comment == '\t' || *end_comment == '\r' || *end_comment == '\n')) {
                                *end_comment = '\0';
                                end_comment--;
                            }
                            if (strlen(comment_ptr) > 0) {
                                strncpy(comment_symbol, comment_ptr, sizeof(comment_symbol) - 1);
                            }
                        }

                        if (strlen(comment_symbol) > 0) {
                            strcpy(symbol, comment_symbol);
                            strcpy(name, comment_symbol);
                        } else {
                            strcpy(symbol, var_name);
                            strcpy(name, var_name);
                        }
                    }
                    
                    if (strlen(addr) > 0 && strcasecmp(addr, "0x000000000000000000000000000000000000dEaD") != 0) {
                        add_discovered_token(addr, symbol, name, decimals);
                        count_parsed++;
                    }
                }
            }
        }
        printf("[SEED] Successfully seeded %d tokens from addresses.sol\n", count_parsed);
        fclose(f);
    }
}

static char *find_batch_result_by_id(const char *batch_resp, int id, char *res_val, size_t max_len) {
    char id_pattern[64];
    snprintf(id_pattern, sizeof(id_pattern), "\"id\":%d", id);
    const char *item = strstr(batch_resp, id_pattern);
    if (!item) {
        snprintf(id_pattern, sizeof(id_pattern), "\"id\": %d", id);
        item = strstr(batch_resp, id_pattern);
    }
    if (!item) return NULL;
    
    // Find the boundaries of the containing object
    const char *start = item;
    while (start > batch_resp && *start != '{') start--;
    const char *end = item;
    while (*end && *end != '}') end++;
    
    // Locate the result field inside this specific object
    const char *res_ptr = strstr(start, "\"result\"");
    if (res_ptr && res_ptr < end) {
        const char *colon = strchr(res_ptr, ':');
        if (colon && colon < end) {
            const char *val_start = strchr(colon, '"');
            if (val_start && val_start < end) {
                val_start++;
                const char *val_end = strchr(val_start, '"');
                if (val_end && val_end <= end) {
                    size_t len = val_end - val_start;
                    if (len >= max_len) len = max_len - 1;
                    strncpy(res_val, val_start, len);
                    res_val[len] = '\0';
                    return res_val;
                }
            }
        }
    }
    return NULL;
}

int tsfi_pulse_get_token_holders_json(const char *token_addr, char *out_buf, size_t max_len, bool force_refresh) {
    char token_lower[64];
    strncpy(token_lower, token_addr, sizeof(token_lower) - 1);
    token_lower[sizeof(token_lower) - 1] = '\0';
    for (int i = 0; token_lower[i]; i++) {
        if (token_lower[i] >= 'A' && token_lower[i] <= 'Z') {
            token_lower[i] = token_lower[i] + 32;
        }
    }

    char cache_path[256];
    snprintf(cache_path, sizeof(cache_path), "assets/holders_%s.dat.bin", token_lower);

    struct stat st;
    if (!force_refresh && stat(cache_path, &st) == 0) {
        time_t now = time(NULL);
        if (now - st.st_mtime < 15 * 24 * 3600) {
            FILE *f = fopen(cache_path, "r");
            if (f) {
                size_t n = fread(out_buf, 1, max_len - 1, f);
                out_buf[n] = '\0';
                fclose(f);
                if (n > 0) {
                    return 1;
                }
            }
        }
    }

    char *logs_buf = malloc(524288);
    if (!logs_buf) return 0;
    
    typedef struct {
        char address[64];
        double balance;
        bool is_contract;
        char name[128];
        char symbol[64];
        bool is_lp;
        char token0[64];
        char token0_sym[64];
        char token1[64];
        char token1_sym[64];
    } HolderInfo;
    
    HolderInfo holders[128];
    int holders_count = 0;
    
    uint64_t dec = 18;
    char sym[64], nam[128];
    resolve_token(token_lower, sym, nam, &dec);
    
    char *main_source = NULL;
    char *loaded_main_buf = NULL;
    double token_price_usd = 0.04840483;
    double price_pls = tsfi_pulse_get_price_in_pls(token_lower);
    if (price_pls > 0.0) {
        double dai_pls = tsfi_pulse_get_price_in_pls("0xefd766ccb38eaf1dfd701853bfce31359239f305");
        if (dai_pls > 0.0) {
            token_price_usd = price_pls / dai_pls;
        }
    }
    
    int page = 1;
    bool keep_going = true;
    extern bool tsfi_pulse_explorer_get_holders_page(const char *token_addr, int page, char *out_buffer, size_t out_max_len);
    
    while (keep_going && holders_count < 128) {
        keep_going = false;
        if (tsfi_pulse_explorer_get_holders_page(token_lower, page, logs_buf, 524288)) {
            char *ptr = logs_buf;
            int page_added = 0;
            double page_min_value_usd = 999999999.0;
            
            while ((ptr = strstr(ptr, "\"address\":\""))) {
                ptr += 11;
                char *end_addr = strchr(ptr, '"');
                if (end_addr && (end_addr - ptr) < 64) {
                    char h_addr[64];
                    strncpy(h_addr, ptr, end_addr - ptr);
                    h_addr[end_addr - ptr] = '\0';
                    
                    char *val_ptr = strstr(end_addr, "\"value\":\"");
                    if (!val_ptr) {
                        val_ptr = strstr(end_addr, "\"balance\":");
                    }
                    if (val_ptr) {
                        if (strncmp(val_ptr, "\"value\":", 8) == 0) {
                            val_ptr += 9;
                        } else {
                            val_ptr += 10;
                        }
                        char *end_val = strchr(val_ptr, '"');
                        char val_str[64];
                        if (end_val) {
                            if (end_val - val_ptr < 64) {
                                strncpy(val_str, val_ptr, end_val - val_ptr);
                                val_str[end_val - val_ptr] = '\0';
                            } else {
                                strcpy(val_str, "0");
                            }
                        } else {
                            int l = 0;
                            while (val_ptr[l] && val_ptr[l] != ',' && val_ptr[l] != '}' && val_ptr[l] != ']' && l < 60) {
                                val_str[l] = val_ptr[l];
                                l++;
                            }
                            val_str[l] = '\0';
                        }
                        
                        double raw_balance = strtod(val_str, NULL);
                        double balance = raw_balance / pow(10, dec);
                        double holder_value_usd = balance * token_price_usd;
                        
                        if (holder_value_usd < page_min_value_usd) {
                            page_min_value_usd = holder_value_usd;
                        }
                        
                        bool already = false;
                        for (int i = 0; i < holders_count; i++) {
                            if (strcasecmp(holders[i].address, h_addr) == 0) {
                                already = true;
                                break;
                            }
                        }
                        
                        if (!already && holders_count < 128) {
                            strcpy(holders[holders_count].address, h_addr);
                            holders[holders_count].balance = balance;
                            holders[holders_count].is_contract = false;
                            holders[holders_count].name[0] = '\0';
                            holders[holders_count].symbol[0] = '\0';
                            holders[holders_count].is_lp = false;
                            holders[holders_count].token0[0] = '\0';
                            holders[holders_count].token0_sym[0] = '\0';
                            holders[holders_count].token1[0] = '\0';
                            holders[holders_count].token1_sym[0] = '\0';
                            holders_count++;
                            page_added++;
                        }
                    }
                }
                ptr = end_addr;
            }
            
            if (page_added > 0 && page_min_value_usd >= 100.0) {
                keep_going = true;
                page++;
            }
        } else {
            char fallback_path[256];
            snprintf(fallback_path, sizeof(fallback_path), "tmp/holders_%s.json", token_lower);
            FILE *f_fall = fopen(fallback_path, "r");
            if (!f_fall) {
                snprintf(fallback_path, sizeof(fallback_path), "assets/holders_%s.dat.bin", token_lower);
                f_fall = fopen(fallback_path, "r");
            }
            if (f_fall) {
                loaded_main_buf = malloc(524288);
                if (loaded_main_buf) {
                    size_t n = fread(loaded_main_buf, 1, 524287, f_fall);
                    loaded_main_buf[n] = '\0';
                    main_source = loaded_main_buf;
                }
                fclose(f_fall);
            }
            
            if (main_source) {
                char *ptr = main_source;
                while ((ptr = strstr(ptr, "\"address\":\""))) {
                    ptr += 11;
                    char *end_addr = strchr(ptr, '"');
                    if (end_addr && (end_addr - ptr) < 64) {
                        char h_addr[64];
                        strncpy(h_addr, ptr, end_addr - ptr);
                        h_addr[end_addr - ptr] = '\0';
                        
                        char *val_ptr = strstr(end_addr, "\"value\":\"");
                        if (!val_ptr) {
                            val_ptr = strstr(end_addr, "\"balance\":");
                        }
                        if (val_ptr) {
                            if (strncmp(val_ptr, "\"value\":", 8) == 0) {
                                val_ptr += 9;
                            } else {
                                val_ptr += 10;
                            }
                            char *end_val = strchr(val_ptr, '"');
                            char val_str[64];
                            if (end_val) {
                                if (end_val - val_ptr < 64) {
                                    strncpy(val_str, val_ptr, end_val - val_ptr);
                                    val_str[end_val - val_ptr] = '\0';
                                } else {
                                    strcpy(val_str, "0");
                                }
                            } else {
                                int l = 0;
                                while (val_ptr[l] && val_ptr[l] != ',' && val_ptr[l] != '}' && val_ptr[l] != ']' && l < 60) {
                                    val_str[l] = val_ptr[l];
                                    l++;
                                }
                                val_str[l] = '\0';
                            }
                            
                            double balance = strtod(val_str, NULL);
                            
                            bool already = false;
                            for (int i = 0; i < holders_count; i++) {
                                if (strcasecmp(holders[i].address, h_addr) == 0) {
                                    already = true;
                                    break;
                                }
                            }
                            
                            if (!already && holders_count < 128) {
                                strcpy(holders[holders_count].address, h_addr);
                                holders[holders_count].balance = balance;
                                holders[holders_count].is_contract = false;
                                holders[holders_count].name[0] = '\0';
                                holders[holders_count].symbol[0] = '\0';
                                holders[holders_count].is_lp = false;
                                holders[holders_count].token0[0] = '\0';
                                holders[holders_count].token0_sym[0] = '\0';
                                holders[holders_count].token1[0] = '\0';
                                holders[holders_count].token1_sym[0] = '\0';
                                holders_count++;
                            }
                        }
                    }
                    ptr = end_addr;
                }
            }
            break;
        }
    }
    
    const char *factories[] = {
        "0x1715a3E4A142d8b698131108995174F37aEBA10D",
        "0x29ea7545def87022badc76323f373ea1e707c523",
        NULL
    };
    
    bool self_is_con = false;
    bool self_is_lp = false;
    get_contract_cache_status(token_lower, &self_is_con, &self_is_lp, NULL, NULL, NULL, NULL);
    
    if (g_mcp_rdbms_table && !self_is_lp) {
        for (uint32_t j = 0; j < g_mcp_rdbms_table->count; j++) {
            const char *other_token = g_mcp_rdbms_table->rows[j].address;
            if (strcasecmp(other_token, token_lower) == 0) continue;
            
            for (int f = 0; factories[f] != NULL; f++) {
                char calldata[256];
                
                const char *tA_ptr = token_lower;
                if (strncmp(tA_ptr, "0x", 2) == 0) tA_ptr += 2;
                const char *tB_ptr = other_token;
                if (strncmp(tB_ptr, "0x", 2) == 0) tB_ptr += 2;
                
                snprintf(calldata, sizeof(calldata),
                         "0xe6a43905000000000000000000000000%.40s000000000000000000000000%.40s",
                         tA_ptr, tB_ptr);
                
                char r_buf[1024];
                if (tsfi_pulse_rpc_call(factories[f], calldata, r_buf, sizeof(r_buf))) {
                    char lp_addr[64];
                    decode_abi_address(r_buf, lp_addr);
                    
                    if (strlen(lp_addr) > 0 && strcasecmp(lp_addr, "0x0000000000000000000000000000000000000000") != 0) {
                        bool already = false;
                        for (int i = 0; i < holders_count; i++) {
                            if (strcasecmp(holders[i].address, lp_addr) == 0) {
                                already = true;
                                break;
                            }
                        }
                        
                        if (!already && holders_count < 128) {
                            strcpy(holders[holders_count].address, lp_addr);
                            holders[holders_count].balance = 0.0;
                            holders[holders_count].is_contract = true;
                            strcpy(holders[holders_count].name, "PulseX LP");
                            strcpy(holders[holders_count].symbol, "PLP");
                            holders[holders_count].is_lp = true;
                            strcpy(holders[holders_count].token0, token_lower);
                            strcpy(holders[holders_count].token1, other_token);
                            
                            uint64_t t0_dec = 18;
                            resolve_token(token_lower, holders[holders_count].token0_sym, nam, &t0_dec);
                            uint64_t t1_dec = 18;
                            resolve_token(other_token, holders[holders_count].token1_sym, nam, &t1_dec);
                            
                            holders_count++;
                        }
                    }
                }
            }
        }
    }
    
    for (int i = 0; i < holders_count; i++) {
        if (holders[i].is_lp && holders[i].balance == 0.0) {
            char calldata[128];
            const char *h_ptr = holders[i].address;
            if (strncmp(h_ptr, "0x", 2) == 0) h_ptr += 2;
            snprintf(calldata, sizeof(calldata), "0x70a08231000000000000000000000000%.40s", h_ptr);
            
            char r_buf[1024];
            if (tsfi_pulse_rpc_call(token_lower, calldata, r_buf, sizeof(r_buf))) {
                char *hex_start = r_buf;
                if (strncmp(hex_start, "0x", 2) == 0) hex_start += 2;
                uint64_t high = 0, low = 0;
                if (strlen(hex_start) >= 64) {
                    char temp[17];
                    memcpy(temp, hex_start + 32, 16); temp[16] = '\0';
                    high = strtoull(temp, NULL, 16);
                    memcpy(temp, hex_start + 48, 16); temp[16] = '\0';
                    low = strtoull(temp, NULL, 16);
                } else {
                    low = strtoull(hex_start, NULL, 16);
                }
                double raw_bal = (double)high * 18446744073709551616.0 + (double)low;
                holders[i].balance = raw_bal / pow(10, dec);
            }
        }
    }
    
    if (loaded_main_buf) free(loaded_main_buf);
    free(logs_buf);
    
    for (int i = 0; i < holders_count; i++) {
        bool is_contract = false;
        bool is_lp = false;
        char t0[64] = {0}, t0_sym[64] = {0}, t1[64] = {0}, t1_sym[64] = {0};
        
        bool is_seeded = false;
        if (g_mcp_rdbms_table) {
            for (uint32_t j = 0; j < g_mcp_rdbms_table->count; j++) {
                if (strcasecmp(g_mcp_rdbms_table->rows[j].address, holders[i].address) == 0) {
                    is_seeded = true;
                    break;
                }
            }
        }
        
        if (is_seeded) {
            holders[i].is_contract = true;
        } else if (get_contract_cache_status(holders[i].address, &is_contract, &is_lp, t0, t0_sym, t1, t1_sym)) {
            holders[i].is_contract = is_contract;
            holders[i].is_lp = is_lp;
            if (is_lp) {
                strcpy(holders[i].token0, t0);
                strcpy(holders[i].token0_sym, t0_sym);
                strcpy(holders[i].token1, t1);
                strcpy(holders[i].token1_sym, t1_sym);
            }
        }
    }
    
    char *batch_payload = malloc(65536);
    char *batch_response = malloc(524288);
    if (batch_payload && batch_response) {
        size_t b_len = 0;
        b_len += snprintf(batch_payload + b_len, 65536 - b_len, "[");
        int batch_count = 0;
        for (int i = 0; i < holders_count; i++) {
            bool already_determined = false;
            bool dummy_is_con = false, dummy_is_lp = false;
            if (get_contract_cache_status(holders[i].address, &dummy_is_con, &dummy_is_lp, NULL, NULL, NULL, NULL)) {
                already_determined = true;
            }
            if (g_mcp_rdbms_table) {
                for (uint32_t j = 0; j < g_mcp_rdbms_table->count; j++) {
                    if (strcasecmp(g_mcp_rdbms_table->rows[j].address, holders[i].address) == 0) {
                        already_determined = true;
                        break;
                    }
                }
            }
            if (!already_determined && !holders[i].is_contract) {
                b_len += snprintf(batch_payload + b_len, 65536 - b_len,
                                   "%s{\"jsonrpc\":\"2.0\",\"method\":\"eth_getCode\",\"params\":[\"%s\",\"latest\"],\"id\":%d}",
                                   (batch_count > 0) ? "," : "",
                                   holders[i].address,
                                   i);
                batch_count++;
            }
        }
        b_len += snprintf(batch_payload + b_len, 65536 - b_len, "]");
        
        if (batch_count > 0 && tsfi_pulse_rpc_exec_raw_body(batch_payload, batch_response, 524288)) {
            for (int i = 0; i < holders_count; i++) {
                bool already_determined = false;
                bool dummy_is_con = false, dummy_is_lp = false;
                if (get_contract_cache_status(holders[i].address, &dummy_is_con, &dummy_is_lp, NULL, NULL, NULL, NULL)) {
                    already_determined = true;
                }
                if (!already_determined && !holders[i].is_contract) {
                    char res_val[256] = {0};
                    if (find_batch_result_by_id(batch_response, i, res_val, sizeof(res_val))) {
                        if (strncmp(res_val, "0x", 2) == 0 && strlen(res_val) > 2) {
                            holders[i].is_contract = true;
                            set_contract_cache_status(holders[i].address, true, false, NULL, NULL, NULL, NULL);
                        } else {
                            set_contract_cache_status(holders[i].address, false, false, NULL, NULL, NULL, NULL);
                        }
                    }
                }
            }
        }
        free(batch_payload);
        free(batch_response);
    }
    for (int i = 0; i < holders_count; i++) {
        if (strcasecmp(holders[i].address, "0x000000000000000000000000000000000000dEaD") == 0 ||
            strcasecmp(holders[i].address, "0x0000000000000000000000000000000000000000") == 0) {
            holders[i].is_contract = true;
            strcpy(holders[i].symbol, "BURN");
            strcpy(holders[i].name, "Burn Address");
            continue;
        }
        if (holders[i].is_contract) {
            bool cache_is_lp = false;
            bool cache_is_con = false;
            char t0[64] = {0}, t0_sym[64] = {0}, t1[64] = {0}, t1_sym[64] = {0};
            if (get_contract_cache_status(holders[i].address, &cache_is_con, &cache_is_lp, t0, t0_sym, t1, t1_sym) && cache_is_con) {
                holders[i].is_lp = cache_is_lp;
                if (cache_is_lp) {
                    strcpy(holders[i].token0, t0);
                    strcpy(holders[i].token0_sym, t0_sym);
                    strcpy(holders[i].token1, t1);
                    strcpy(holders[i].token1_sym, t1_sym);
                    uint64_t c_dec = 18;
                    resolve_token(holders[i].address, holders[i].symbol, holders[i].name, &c_dec);
                    continue;
                }
            }
            
            uint64_t c_dec = 18;
            resolve_token(holders[i].address, holders[i].symbol, holders[i].name, &c_dec);
            
            char r_buf[1024];
            bool resolved_lp = false;
            if (tsfi_pulse_rpc_call(holders[i].address, "0x0dfe1681", r_buf, sizeof(r_buf))) {
                char t0_addr[64];
                decode_abi_address(r_buf, t0_addr);
                if (strlen(t0_addr) > 0 && strcmp(t0_addr, "0x0000000000000000000000000000000000000000") != 0) {
                    if (tsfi_pulse_rpc_call(holders[i].address, "0xd21225a3", r_buf, sizeof(r_buf)) ||
                        tsfi_pulse_rpc_call(holders[i].address, "0xd21220a7", r_buf, sizeof(r_buf))) {
                        char t1_addr[64];
                        decode_abi_address(r_buf, t1_addr);
                        if (strlen(t1_addr) > 0 && strcmp(t1_addr, "0x0000000000000000000000000000000000000000") != 0) {
                            holders[i].is_lp = true;
                            strcpy(holders[i].token0, t0_addr);
                            strcpy(holders[i].token1, t1_addr);
                            
                            char t0_nam[128];
                            uint64_t t0_dec = 18;
                            resolve_token(t0_addr, holders[i].token0_sym, t0_nam, &t0_dec);
                            
                            char t1_nam[128];
                            uint64_t t1_dec = 18;
                            resolve_token(t1_addr, holders[i].token1_sym, t1_nam, &t1_dec);
                            
                            set_contract_cache_status(holders[i].address, true, true, t0_addr, holders[i].token0_sym, t1_addr, holders[i].token1_sym);
                            resolved_lp = true;
                        }
                    }
                }
            }
            if (!resolved_lp) {
                set_contract_cache_status(holders[i].address, true, false, NULL, NULL, NULL, NULL);
            }
        }
    }
    
    for (int i = 0; i < holders_count - 1; i++) {
        for (int j = i + 1; j < holders_count; j++) {
            if (holders[i].balance < holders[j].balance) {
                HolderInfo temp = holders[i];
                holders[i] = holders[j];
                holders[j] = temp;
            }
        }
    }
    
    size_t offset = 0;
    offset += snprintf(out_buf + offset, max_len - offset, "{\"token\":\"%s\",\"holders\":[", token_lower);
    for (int i = 0; i < holders_count; i++) {
        offset += snprintf(out_buf + offset, max_len - offset,
                           "%s{\"address\":\"%s\",\"balance\":%.8f,\"is_contract\":%s"
                           ",\"name\":\"%s\",\"symbol\":\"%s\",\"is_lp\":%s"
                           ",\"token0\":\"%s\",\"token0_symbol\":\"%s\""
                           ",\"token1\":\"%s\",\"token1_symbol\":\"%s\"}",
                           (i > 0) ? "," : "",
                           holders[i].address,
                           holders[i].balance,
                           holders[i].is_contract ? "true" : "false",
                           holders[i].name,
                           holders[i].symbol,
                           holders[i].is_lp ? "true" : "false",
                           holders[i].token0,
                           holders[i].token0_sym,
                           holders[i].token1,
                           holders[i].token1_sym);
        if (offset >= max_len - 512) break;
    }
    offset += snprintf(out_buf + offset, max_len - offset, "]}");

    FILE *f_cache = fopen(cache_path, "w");
    if (f_cache) {
        fwrite(out_buf, 1, strlen(out_buf), f_cache);
        fclose(f_cache);
    }

    return 1;
}

uint32_t tsfi_block_monitor_get_discovered_tokens_count(void) {
    if (g_mcp_rdbms_table) return g_mcp_rdbms_table->count;
    return 0;
}

void tsfi_block_monitor_get_discovered_token(uint32_t idx, char *out_addr, char *out_symbol, char *out_name) {
    if (g_mcp_rdbms_table && idx < g_mcp_rdbms_table->count) {
        strcpy(out_addr, g_mcp_rdbms_table->rows[idx].address);
        strcpy(out_symbol, g_mcp_rdbms_table->rows[idx].symbol);
        strcpy(out_name, g_mcp_rdbms_table->rows[idx].name);
    }
}

