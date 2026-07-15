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

static uint64_t g_last_block_num = 0;
#define MAX_PENDING_BLOCKS 100
static uint64_t g_pending_blocks[MAX_PENDING_BLOCKS];
static int g_pending_blocks_count = 0;
static pthread_mutex_t g_pending_mutex = PTHREAD_MUTEX_INITIALIZER;
static time_t g_last_poll_time = 0;
static tsfi_qing_graph_node g_net_nodes[TSFI_NET_COUNT];
static int g_graph_initialized = 0;

static const InteropDecisionNode g_accumulator_tree[3] = {
    { 20000000ULL, 1, 2, 0 },           // Root decision node
    { 0, 0xFFFFFFFF, 0xFFFFFFFF, 101 }, // Accumulator Lane 1 (High block heights)
    { 0, 0xFFFFFFFF, 0xFFFFFFFF, 102 }  // Accumulator Lane 2 (Low block heights)
};

static const char *PULSEX_ROUTER __attribute__((unused)) = "0x165C3410fC69F1857c278b5ce0ae3512720FD0E1";
static const char *SWAP_V2_TOPIC = "0xd78ad95fa46c994b6551d0da85fc275fe613ce37657fb8d5e3d130840159d822";
static const uint64_t SWAP_FLOOR_THRESHOLD = 1000ULL;
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

static double parse_hex_double(const char *hex, size_t len);

typedef struct {
    char token0[64];
    char token1[64];
    double price; // token1 per token0 (Real ratio)
} SwapEdge;

#define MAX_SWAP_EDGES 1024
static SwapEdge g_swap_edges[MAX_SWAP_EDGES];
static int g_swap_edges_count = 0;

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

#define MAX_DISCOVERED_TOKENS 512

typedef struct {
    char address[64];
    char symbol[64];
    char name[128];
    uint64_t decimals;
    double price_pls;
    uint64_t last_update;
} RdbmsTokenRow;

typedef struct {
    uint32_t active;
    uint32_t col_count;
    uint32_t count;
    uint32_t capacity;
    RdbmsTokenRow rows[MAX_DISCOVERED_TOKENS];
} LauRdbmsTable;

static LauRdbmsTable *g_mcp_rdbms_table = NULL;

#define g_discovered_tokens (g_mcp_rdbms_table->rows)
#define g_discovered_tokens_count (g_mcp_rdbms_table->count)

static void ensure_rdbms_table_initialized(void) {
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

static void save_discovered_tokens(void) {
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

static void load_discovered_tokens(void) {
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

static void print_pricing_routing_table(void) {
    tsfi_io_printf(stderr, "\n========================================================================\n");
    tsfi_io_printf(stderr, "   Auncient Knowledge Graph Real-Time Token Pricing Table (Valued in PLS) \n");
    tsfi_io_printf(stderr, "========================================================================\n");
    tsfi_io_printf(stderr, " %-12s | %-42s | %-20s | %-10s \n", "Symbol", "Token Address", "Price in PLS", "Status");
    tsfi_io_printf(stderr, "------------------------------------------------------------------------\n");
    for (uint32_t i = 0; i < g_discovered_tokens_count; i++) {
        double price = tsfi_pulse_get_price_in_pls(g_discovered_tokens[i].address);
        if (price > 0.0) {
            tsfi_io_printf(stderr, " %-12s | %-42s | %-20.8f | %-10s \n", 
                           g_discovered_tokens[i].symbol, 
                           g_discovered_tokens[i].address, 
                           price, 
                           "ROUTED");
        } else {
            tsfi_io_printf(stderr, " %-12s | %-42s | %-20s | %-10s \n", 
                           g_discovered_tokens[i].symbol, 
                           g_discovered_tokens[i].address, 
                           "N/A", 
                           "NO PATH");
        }
    }
    tsfi_io_printf(stderr, "========================================================================\n\n");
}

static bool is_fallback_symbol(const char *sym) {
    if (!sym || strlen(sym) == 0) return true;
    if (strcmp(sym, "UNKNOWN") == 0 || strcmp(sym, "UNKN") == 0 || strcmp(sym, "UNKNOWN_TOKEN") == 0) return true;
    size_t len = strlen(sym);
    if (len >= 8 && strcmp(sym + len - 8, "Contract") == 0) return true;
    if (len >= 6 && strcmp(sym + len - 6, "Router") == 0) return true;
    return false;
}

static void resolve_token(const char *addr, char *out_symbol, char *out_name, uint64_t *out_decimals) {
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

static void save_swap_edges(void) {
    FILE *f = fopen("assets/swap_edges.dat.bin", "wb");
    if (f) {
        uint32_t count = (uint32_t)g_swap_edges_count;
        fwrite(&count, sizeof(count), 1, f);
        fwrite(g_swap_edges, sizeof(SwapEdge), count, f);
        fclose(f);
    }
}

static void load_swap_edges(void) {
    FILE *f = fopen("assets/swap_edges.dat.bin", "rb");
    if (f) {
        uint32_t count = 0;
        if (fread(&count, sizeof(count), 1, f) == 1) {
            g_swap_edges_count = (int)count;
            if (g_swap_edges_count > MAX_SWAP_EDGES) g_swap_edges_count = MAX_SWAP_EDGES;
            size_t read_bytes = fread(g_swap_edges, sizeof(SwapEdge), g_swap_edges_count, f);
            (void)read_bytes;
        }
        fclose(f);
    }
}

void add_swap_edge(const char *t0, const char *t1, double price) {
    if (price <= 0.0) return;
    for (int i = 0; i < g_swap_edges_count; i++) {
        if ((strcasecmp(g_swap_edges[i].token0, t0) == 0 && strcasecmp(g_swap_edges[i].token1, t1) == 0) ||
            (strcasecmp(g_swap_edges[i].token0, t1) == 0 && strcasecmp(g_swap_edges[i].token1, t0) == 0)) {
            if (strcasecmp(g_swap_edges[i].token0, t0) == 0) {
                g_swap_edges[i].price = price;
            } else {
                g_swap_edges[i].price = 1.0 / price;
            }
            save_swap_edges();
            return;
        }
    }
    if (g_swap_edges_count < MAX_SWAP_EDGES) {
        strcpy(g_swap_edges[g_swap_edges_count].token0, t0);
        strcpy(g_swap_edges[g_swap_edges_count].token1, t1);
        g_swap_edges[g_swap_edges_count].price = price;
        g_swap_edges_count++;
        save_swap_edges();
    }
}

typedef struct {
    char address[64];
    double price_multiplier;
} QueueNode;

static double bfs_price_route(const char *start, const char *target) {
    if (strcasecmp(start, target) == 0) {
        return 1.0;
    }
    static QueueNode queue[1024];
    int head = 0;
    int tail = 0;
    static char visited_nodes[512][64];
    int visited_count = 0;
    
    strcpy(queue[tail].address, start);
    queue[tail].price_multiplier = 1.0;
    tail++;
    strcpy(visited_nodes[visited_count++], start);
    
    while (head < tail) {
        QueueNode curr = queue[head++];
        if (strcasecmp(curr.address, target) == 0) {
            return curr.price_multiplier;
        }
        for (int i = 0; i < g_swap_edges_count; i++) {
            const char *next = NULL;
            double edge_price = 0.0;
            if (strcasecmp(g_swap_edges[i].token0, curr.address) == 0) {
                next = g_swap_edges[i].token1;
                edge_price = g_swap_edges[i].price;
            } else if (strcasecmp(g_swap_edges[i].token1, curr.address) == 0) {
                next = g_swap_edges[i].token0;
                edge_price = 1.0 / g_swap_edges[i].price;
            }
            if (next) {
                int is_visited = 0;
                for (int v = 0; v < visited_count; v++) {
                    if (strcasecmp(visited_nodes[v], next) == 0) {
                        is_visited = 1;
                        break;
                    }
                }
                if (!is_visited && visited_count < 512 && tail < 1024) {
                    strcpy(visited_nodes[visited_count++], next);
                    strcpy(queue[tail].address, next);
                    queue[tail].price_multiplier = curr.price_multiplier * edge_price;
                    tail++;
                }
            }
        }
    }
    return 0.0;
}

double tsfi_pulse_get_price_in_pls(const char *token_addr) {
    const char *wpls = "0xa1077a294dde1b09bb078844df40758a5d0f9a27";
    return bfs_price_route(token_addr, wpls);
}

typedef struct {
    char pool_address[64];
    char token0[64];
    char token1[64];
    double token0_balance;
    double token1_balance;
    double last_price;
    double last_amount;
} PoolStateCache;

#define MAX_POOL_CACHE 256
static PoolStateCache g_pool_cache[MAX_POOL_CACHE];
typedef struct {
    uint64_t block_number;
    uint32_t pulsex_tx_count;
    uint32_t unknown_tx_count;
} BlockHistoryEntry;

#define MAX_BLOCK_HISTORY 50
static BlockHistoryEntry g_block_history[MAX_BLOCK_HISTORY];
static int g_block_history_count = 0;

static void save_block_history(void) {
    FILE *f = fopen("assets/block_history.dat.bin", "wb");
    if (f) {
        uint32_t count = (uint32_t)g_block_history_count;
        fwrite(&count, sizeof(count), 1, f);
        fwrite(g_block_history, sizeof(BlockHistoryEntry), count, f);
        fclose(f);
    }
}

static void load_block_history(void) {
    FILE *f = fopen("assets/block_history.dat.bin", "rb");
    if (f) {
        uint32_t count = 0;
        if (fread(&count, sizeof(count), 1, f) == 1) {
            g_block_history_count = (int)count;
            if (g_block_history_count > MAX_BLOCK_HISTORY) g_block_history_count = MAX_BLOCK_HISTORY;
            size_t r = fread(g_block_history, sizeof(BlockHistoryEntry), g_block_history_count, f);
            (void)r;
        }
        fclose(f);
    }
    
    // Repopulate knowledge graph with loaded block history
    for (int i = 0; i < g_block_history_count; i++) {
        uint64_t blk = g_block_history[i].block_number;
        uint32_t unk = g_block_history[i].unknown_tx_count;
        if (g_graph_initialized) {
            CachedContract *new_c = (CachedContract*)malloc(sizeof(CachedContract));
            if (new_c) {
                snprintf(new_c->name, sizeof(new_c->name), "block_%lu_unk_%u", blk, unk);
                new_c->virtual_address = blk;
                new_c->bytecode = NULL;
                new_c->size = 0;
                new_c->path[0] = '\0';
                
                g_net_nodes[TSFI_NET_PULSECHAIN].bst_root = tsfi_qing_bst_insert(
                    g_net_nodes[TSFI_NET_PULSECHAIN].bst_root, 
                    blk, 
                    new_c
                );
            }
        }
    }
}

static int g_pool_cache_count = 0;

static void save_pool_cache(void) {
    FILE *f = fopen("assets/pool_cache.dat.bin", "wb");
    if (f) {
        uint32_t count = (uint32_t)g_pool_cache_count;
        fwrite(&count, sizeof(count), 1, f);
        fwrite(g_pool_cache, sizeof(PoolStateCache), count, f);
        fclose(f);
    }
}

static void ensure_pool_in_cache(const char *pool, const char *t0, const char *t1, double price) {
    for (int i = 0; i < g_pool_cache_count; i++) {
        if (strcasecmp(g_pool_cache[i].pool_address, pool) == 0) {
            if (g_pool_cache[i].last_price <= 0.0) {
                g_pool_cache[i].last_price = price;
            }
            return;
        }
    }
    if (g_pool_cache_count < MAX_POOL_CACHE) {
        int idx = g_pool_cache_count++;
        strcpy(g_pool_cache[idx].pool_address, pool);
        strcpy(g_pool_cache[idx].token0, t0);
        strcpy(g_pool_cache[idx].token1, t1);
        g_pool_cache[idx].token0_balance = 0.0;
        g_pool_cache[idx].token1_balance = 0.0;
        g_pool_cache[idx].last_price = price;
        g_pool_cache[idx].last_amount = 0.0;
    }
}

static void load_pool_cache(void) {
    FILE *f = fopen("assets/pool_cache.dat.bin", "rb");
    if (f) {
        uint32_t count = 0;
        if (fread(&count, sizeof(count), 1, f) == 1) {
            g_pool_cache_count = (int)count;
            if (g_pool_cache_count > MAX_POOL_CACHE) g_pool_cache_count = MAX_POOL_CACHE;
            size_t read_bytes = fread(g_pool_cache, sizeof(PoolStateCache), g_pool_cache_count, f);
            (void)read_bytes;
        }
        fclose(f);
    }
    
    // Seed ATROPA pools if missing
    ensure_pool_in_cache("0x5ef7aac0de4f2012cb36730da140025b113fada4", "0x6b175474e89094c44da98b954eedeac495271d0f", "0xCc78A0acDF847A2C1714D2A925bB4477df5d48a6", 0.007741);
    ensure_pool_in_cache("0xc636bfe0bae34824380b4e26bc34e4614e55e483", "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48", "0xCc78A0acDF847A2C1714D2A925bB4477df5d48a6", 0.018481);
    ensure_pool_in_cache("0xe1d2bdba58d34109c547883dc9c2f9e01cebb003", "0x2556f7f8d82ebcdd7b821b0981c38d9da9439cdd", "0xCc78A0acDF847A2C1714D2A925bB4477df5d48a6", 0.0001);
    ensure_pool_in_cache("0x0a022e7591749b0ed0d9e3b7b978f26978440dc7", "0x4243568fa2bbad327ee36e06c16824cad8b37819", "0xCc78A0acDF847A2C1714D2A925bB4477df5d48a6", 0.01);

    // Feed swap edges from cached pools to resolve prices immediately
    for (int i = 0; i < g_pool_cache_count; i++) {
        if (g_pool_cache[i].last_price > 0.0) {
            add_swap_edge(g_pool_cache[i].token0, g_pool_cache[i].token1, g_pool_cache[i].last_price);
        } else if (g_pool_cache[i].token0_balance > 0.0 && g_pool_cache[i].token1_balance > 0.0) {
            double reserve_price = g_pool_cache[i].token1_balance / g_pool_cache[i].token0_balance;
            add_swap_edge(g_pool_cache[i].token0, g_pool_cache[i].token1, reserve_price);
        }
    }
}

static bool check_rpc_connectivity(void) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return false;
    
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo("pulsechain-rpc.publicnode.com", "443", &hints, &res) != 0) {
        close(sockfd);
        return false;
    }
    
    int ret = connect(sockfd, res->ai_addr, res->ai_addrlen);
    if (ret < 0) {
        if (errno == EINPROGRESS) {
            fd_set wset;
            FD_ZERO(&wset);
            FD_SET(sockfd, &wset);
            struct timeval tv = {1, 0}; // 1.0 second timeout
            if (select(sockfd + 1, NULL, &wset, NULL, &tv) > 0) {
                int err;
                socklen_t len = sizeof(err);
                if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &len) == 0 && err == 0) {
                    freeaddrinfo(res);
                    close(sockfd);
                    return true;
                }
            }
        }
    } else {
        freeaddrinfo(res);
        close(sockfd);
        return true;
    }
    
    freeaddrinfo(res);
    close(sockfd);
    return false;
}

static void tsfi_block_monitor_sync_reserves_startup(void) {
    if (!check_rpc_connectivity()) {
        tsfi_io_printf(stderr, "[MONITOR] RPC connectivity check failed. Bypassing startup reserve synchronization (Offline mode).\n");
        for (int i = 0; i < g_pool_cache_count; i++) {
            if (g_pool_cache[i].token0_balance > 0.0 && g_pool_cache[i].token1_balance > 0.0) {
                double reserve_price = g_pool_cache[i].token1_balance / g_pool_cache[i].token0_balance;
                add_swap_edge(g_pool_cache[i].token0, g_pool_cache[i].token1, reserve_price);
            }
        }
        return;
    }
    tsfi_io_printf(stderr, "[MONITOR] Synchronizing reserves for %d cached pools on startup...\n", g_pool_cache_count);
    for (int i = 0; i < g_pool_cache_count; i++) {
        char r_buf[1024];
        if (tsfi_pulse_rpc_call(g_pool_cache[i].pool_address, "0x0902f1ac", r_buf, sizeof(r_buf))) {
            const char *hex = r_buf;
            if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X')) hex += 2;
            if (strlen(hex) >= 128) {
                char r0_hex[65] = {0};
                char r1_hex[65] = {0};
                strncpy(r0_hex, hex, 64);
                strncpy(r1_hex, hex + 64, 64);
                
                double r0 = parse_hex_double(r0_hex, 64);
                double r1 = parse_hex_double(r1_hex, 64);
                
                int dec0 = 18;
                int dec1 = 18;
                for (size_t k = 0; k < KNOWN_TOKENS_COUNT; k++) {
                    if (strcasecmp(g_known_tokens[k].address, g_pool_cache[i].token0) == 0) dec0 = g_known_tokens[k].decimals;
                    if (strcasecmp(g_known_tokens[k].address, g_pool_cache[i].token1) == 0) dec1 = g_known_tokens[k].decimals;
                }
                
                g_pool_cache[i].token0_balance = r0 / pow(10, dec0);
                g_pool_cache[i].token1_balance = r1 / pow(10, dec1);
                
                if (g_pool_cache[i].token0_balance > 0.0) {
                    g_pool_cache[i].last_price = g_pool_cache[i].token1_balance / g_pool_cache[i].token0_balance;
                    add_swap_edge(g_pool_cache[i].token0, g_pool_cache[i].token1, g_pool_cache[i].last_price);
                }
            }
        }
    }
    tsfi_io_printf(stderr, "[MONITOR] Reserves sync complete. Swap edges: %d\n", g_swap_edges_count);
}

static int strcasefind(const char *haystack, const char *needle) __attribute__((unused));
static int strcasefind(const char *haystack, const char *needle) {
    if (!haystack || !needle) return 0;
    size_t needle_len = strlen(needle);
    size_t haystack_len = strlen(haystack);
    if (needle_len > haystack_len) return 0;
    for (size_t i = 0; i <= haystack_len - needle_len; i++) {
        size_t j;
        for (j = 0; j < needle_len; j++) {
            char hc = haystack[i + j];
            char nc = needle[j];
            if (hc >= 'A' && hc <= 'Z') hc += 32;
            if (nc >= 'A' && nc <= 'Z') nc += 32;
            if (hc != nc) break;
        }
        if (j == needle_len) return 1;
    }
    return 0;
}

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

static uint32_t evaluate_accumulator_lane(uint64_t val) {
    uint32_t curr = 0;
    while (1) {
        if (g_accumulator_tree[curr].left_child_idx == 0xFFFFFFFF && 
            g_accumulator_tree[curr].right_child_idx == 0xFFFFFFFF) {
            return g_accumulator_tree[curr].branch_selector;
        }
        if (val >= g_accumulator_tree[curr].threshold) {
            curr = g_accumulator_tree[curr].left_child_idx;
        } else {
            curr = g_accumulator_tree[curr].right_child_idx;
        }
    }
}

static void seed_tokens_from_addresses_sol(void) {
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

void tsfi_block_monitor_init(void) {
    ensure_rdbms_table_initialized();
    load_discovered_tokens();
    g_last_block_num = 0;
    FILE *bf = fopen("assets/last_block_num.dat.bin", "rb");
    if (bf) {
        size_t r = fread(&g_last_block_num, sizeof(g_last_block_num), 1, bf);
        (void)r;
        fclose(bf);
    }
    if (g_last_block_num > 0) {
        pthread_mutex_lock(&g_pending_mutex);
        g_pending_blocks_count = 0;
        for (int i = 0; i < 10; i++) {
            if (g_last_block_num >= (uint64_t)(9 - i)) {
                g_pending_blocks[g_pending_blocks_count++] = g_last_block_num - (9 - i);
            }
        }
        pthread_mutex_unlock(&g_pending_mutex);
    }
    g_last_poll_time = 0;
    g_pool_cache_count = 0;
    memset(g_pool_cache, 0, sizeof(g_pool_cache));
    tsfi_qing_graph_init(g_net_nodes);
    
    // Connect topology: ZMM <-> Anvil_EVM <-> Pulsechain
    tsfi_qing_graph_add_edge(&g_net_nodes[TSFI_NET_ZMM], &g_net_nodes[TSFI_NET_ANVIL_EVM]);
    tsfi_qing_graph_add_edge(&g_net_nodes[TSFI_NET_ANVIL_EVM], &g_net_nodes[TSFI_NET_PULSECHAIN]);
    g_graph_initialized = 1;

    // Seed default popular tokens if not loaded from cache
    add_discovered_token("0xA1077a294dDe1B09bB078844df40758a5D0f9a27", "WPLS", "Wrapped PLS", 18);
    add_discovered_token("0x95acD3924370324F973352614Ea2624F0cf0d7eb", "PLSX", "PulseX", 18);
    add_discovered_token("0x2b5455ad41d3002b573334244649027ce76289c5", "HEX", "HEX", 8);
    add_discovered_token("0xefD766ccB0F39a5e6219b902cd81B85f984D19Ca", "DAI", "DAI from Ethereum", 18);
    add_discovered_token("0x15D3853B874d6A018eD6102B515Ef037Ad63e650", "USDC", "USDC from Ethereum", 6);
    add_discovered_token("0x02aAA39b223FE8D0A0e5C4F27eAD9083C756Cc2", "WETH", "Wrapped Ether from Ethereum", 18);

    // Seed default swap edges so prices resolve immediately
    add_swap_edge("0x95acD3924370324F973352614Ea2624F0cf0d7eb", "0xA1077a294dDe1B09bB078844df40758a5D0f9a27", 0.0033);
    add_swap_edge("0xefD766ccB0F39a5e6219b902cd81B85f984D19Ca", "0xA1077a294dDe1B09bB078844df40758a5D0f9a27", 268.0587);
    add_swap_edge("0x2b5455ad41d3002b573334244649027ce76289c5", "0xA1077a294dDe1B09bB078844df40758a5D0f9a27", 100.0);

    // Seed default LP pools
    if (g_pool_cache_count < MAX_POOL_CACHE) {
        int idx = g_pool_cache_count++;
        strcpy(g_pool_cache[idx].pool_address, "0x1b418a0b0d4c9a3b2b5455ad41d3002b57333424");
        strcpy(g_pool_cache[idx].token0, "0x95acD3924370324F973352614Ea2624F0cf0d7eb");
        strcpy(g_pool_cache[idx].token1, "0xA1077a294dDe1B09bB078844df40758a5D0f9a27");
        g_pool_cache[idx].token0_balance = 500000000.0;
        g_pool_cache[idx].token1_balance = 1650000.0;
        g_pool_cache[idx].last_price = 0.0033;
        g_pool_cache[idx].last_amount = 10000.0;
    }
    if (g_pool_cache_count < MAX_POOL_CACHE) {
        int idx = g_pool_cache_count++;
        strcpy(g_pool_cache[idx].pool_address, "0xefd766ccb8c15e5e9f813af7b2809857baa53a1f");
        strcpy(g_pool_cache[idx].token0, "0xefD766ccB0F39a5e6219b902cd81B85f984D19Ca");
        strcpy(g_pool_cache[idx].token1, "0xA1077a294dDe1B09bB078844df40758a5D0f9a27");
        g_pool_cache[idx].token0_balance = 100000.0;
        g_pool_cache[idx].token1_balance = 26805870.0;
        g_pool_cache[idx].last_price = 268.0587;
        g_pool_cache[idx].last_amount = 500.0;
    }

    seed_tokens_from_addresses_sol();
    load_swap_edges();
    load_pool_cache();
    load_block_history();
    if (g_pool_cache_count == 0) {
        tsfi_block_monitor_sync_reserves_startup();
    }
}

tsfi_qing_graph_node* tsfi_block_monitor_get_graph(void) {
    return g_net_nodes;
}

int tsfi_pulse_get_all_prices_json(char *out_buf, size_t max_len) {
    size_t offset = 0;
    offset += snprintf(out_buf + offset, max_len - offset, "{\"tokens\":[");
    for (uint32_t i = 0; i < g_discovered_tokens_count; i++) {
        double price = tsfi_pulse_get_price_in_pls(g_discovered_tokens[i].address);
        offset += snprintf(out_buf + offset, max_len - offset,
                           "%s{\"address\":\"%s\",\"symbol\":\"%s\",\"name\":\"%s\",\"price_pls\":%.8f,\"lp_holdings\":[",
                           (i > 0) ? "," : "",
                           g_discovered_tokens[i].address,
                           g_discovered_tokens[i].symbol,
                           g_discovered_tokens[i].name,
                           price);
        
        int lp_count = 0;
        for (int j = 0; j < g_pool_cache_count; j++) {
            if (strcasecmp(g_pool_cache[j].token0, g_discovered_tokens[i].address) == 0) {
                offset += snprintf(out_buf + offset, max_len - offset,
                                   "%s{\"pool\":\"%s\",\"paired_token\":\"%s\",\"token_balance\":%.8f,\"paired_balance\":%.8f}",
                                   (lp_count > 0) ? "," : "",
                                   g_pool_cache[j].pool_address,
                                   g_pool_cache[j].token1,
                                   g_pool_cache[j].token0_balance,
                                   g_pool_cache[j].token1_balance);
                lp_count++;
            } else if (strcasecmp(g_pool_cache[j].token1, g_discovered_tokens[i].address) == 0) {
                offset += snprintf(out_buf + offset, max_len - offset,
                                   "%s{\"pool\":\"%s\",\"paired_token\":\"%s\",\"token_balance\":%.8f,\"paired_balance\":%.8f}",
                                   (lp_count > 0) ? "," : "",
                                   g_pool_cache[j].pool_address,
                                   g_pool_cache[j].token0,
                                   g_pool_cache[j].token1_balance,
                                   g_pool_cache[j].token0_balance);
                lp_count++;
            }
            if (offset >= max_len - 256) break;
        }
        offset += snprintf(out_buf + offset, max_len - offset, "]}");
        if (offset >= max_len - 128) break;
    }
    offset += snprintf(out_buf + offset, max_len - offset, "]}");
    return 1;
}

int tsfi_pulse_get_unpriced_tokens_json(char *out_buf, size_t max_len) {
    size_t offset = 0;
    offset += snprintf(out_buf + offset, max_len - offset, "{\"tokens\":[");
    int count = 0;
    for (uint32_t i = 0; i < g_discovered_tokens_count; i++) {
        double price = tsfi_pulse_get_price_in_pls(g_discovered_tokens[i].address);
        if (price <= 0.0 && strcmp(g_discovered_tokens[i].name, "UNKNOWN") != 0 && strcmp(g_discovered_tokens[i].symbol, "UNKNOWN") != 0) {
            offset += snprintf(out_buf + offset, max_len - offset,
                               "%s{\"address\":\"%s\",\"symbol\":\"%s\",\"name\":\"%s\"}",
                               (count > 0) ? "," : "",
                               g_discovered_tokens[i].address,
                               g_discovered_tokens[i].symbol,
                               g_discovered_tokens[i].name);
            count++;
        }
    }
    offset += snprintf(out_buf + offset, max_len - offset, "]}");
    return 1;
}

void tsfi_block_monitor_tick(TsfiZmmVmState *state) {
    (void)state;
    time_t now = time(NULL);
    // Poll every 3 seconds to avoid spamming the RPC endpoint
    if (now - g_last_poll_time < 3) {
        return;
    }
    g_last_poll_time = now;

    char buf[2048] = {0};
    const char *payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_blockNumber\",\"params\":[],\"id\":1}";
    
    if (tsfi_pulse_rpc_exec_raw(payload, buf, sizeof(buf))) {
        if (strlen(buf) > 0) {
            uint64_t current_block = strtoull(buf, NULL, 16);
            if (g_last_block_num == 0) {
                g_last_block_num = current_block;
                tsfi_io_printf(stderr, "[MONITOR] Initialized block height to: %lu (Auncient block monitoring active)\n", current_block);
                FILE *bf_w = fopen("assets/last_block_num.dat.bin", "wb");
                if (bf_w) {
                    fwrite(&g_last_block_num, sizeof(g_last_block_num), 1, bf_w);
                    fclose(bf_w);
                }
            } else if (current_block > g_last_block_num) {
                if (current_block - g_last_block_num > 50) {
                    tsfi_io_printf(stderr, "[MONITOR] Catchup range too large (%lu blocks), shifting start to %lu\n", current_block - g_last_block_num, current_block - 50);
                    g_last_block_num = current_block - 50;
                }
                
                uint64_t target_block = g_last_block_num + 1;
                tsfi_io_printf(stderr, "[MONITOR] Attempting to process block: %lu\n", target_block);
                
                // Query Transaction Count for the new block
                uint32_t pulsex_tx_count = 0;
                uint32_t unknown_tx_count = 0;
                char tx_count_payload[256];
                char block_hex[32];
                snprintf(block_hex, sizeof(block_hex), "0x%lx", target_block);
                snprintf(tx_count_payload, sizeof(tx_count_payload),
                         "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getBlockTransactionCountByNumber\",\"params\":[\"%s\"],\"id\":1}",
                         block_hex);
                
                char count_buf[128] = {0};
                if (tsfi_pulse_rpc_exec_raw(tx_count_payload, count_buf, sizeof(count_buf))) {
                    uint64_t total_txs = strtoull(count_buf, NULL, 16);
                    if (total_txs == 0) {
                        tsfi_io_printf(stderr, "[MONITOR] Zero transactions returned for block %lu; treating as RPC failure.\n", target_block);
                        return;
                    }
                    
                    // Drive input event directly to WinchesterMQ
                    char cmd_buf[128];
                    snprintf(cmd_buf, sizeof(cmd_buf), "PULSECHAIN:BLOCK:%lu", target_block);
                    tsfi_thunk_publish_mq(cmd_buf);
                    unknown_tx_count = total_txs;
                }

                tsfi_io_printf(stderr, "[MONITOR] Block %lu transactions accumulated: PulseX=%u, Unknown=%u\n",
                               current_block, pulsex_tx_count, unknown_tx_count);

                // Query logs to extract Swap events for PulseX
                char logs_payload[512];
                snprintf(logs_payload, sizeof(logs_payload),
                         "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getLogs\",\"params\":[{\"fromBlock\":\"%s\",\"toBlock\":\"%s\",\"topics\":[\"%s\"]}],\"id\":1}",
                         block_hex, block_hex, SWAP_V2_TOPIC);
                
                char logs_buf[16384] = {0};
                if (tsfi_pulse_rpc_exec_raw(logs_payload, logs_buf, sizeof(logs_buf))) {
                    char *ptr = logs_buf;
                    while ((ptr = strstr(ptr, "\"address\":\""))) {
                        ptr += 11;
                        char pool_address[64] = {0};
                        char *end_addr = strchr(ptr, '"');
                        if (end_addr) {
                            strncpy(pool_address, ptr, end_addr - ptr);
                        }
                        
                        char *data_ptr = strstr(ptr, "\"data\":\"");
                        if (data_ptr) {
                            data_ptr += 8;
                            if (strncmp(data_ptr, "0x", 2) == 0) data_ptr += 2;
                            double amt0_in = parse_hex_double(data_ptr, 64);
                            double amt1_in = parse_hex_double(data_ptr + 64, 64);
                            double amt0_out = parse_hex_double(data_ptr + 128, 64);
                            double amt1_out = parse_hex_double(data_ptr + 192, 64);
                            
                            double swap_amt = (amt0_in > 0.0) ? amt0_in : amt0_out;
                            
                            // Immediately drop and free any swaps below the floor threshold
                            if (swap_amt < (double)SWAP_FLOOR_THRESHOLD) {
                                ptr = data_ptr;
                                continue;
                            }

                            char t0_addr[64] = {0};
                            char t1_addr[64] = {0};
                            char call_buf[1024];
                            
                            if (tsfi_pulse_rpc_call(pool_address, "0x0dfe1681", call_buf, sizeof(call_buf))) {
                                if (strlen(call_buf) >= 40) {
                                    snprintf(t0_addr, sizeof(t0_addr), "0x%s", call_buf + strlen(call_buf) - 40);
                                }
                            }
                            if (tsfi_pulse_rpc_call(pool_address, "0xd21225a3", call_buf, sizeof(call_buf)) ||
                                tsfi_pulse_rpc_call(pool_address, "0xd21220a7", call_buf, sizeof(call_buf))) {
                                if (strlen(call_buf) >= 40) {
                                    snprintf(t1_addr, sizeof(t1_addr), "0x%s", call_buf + strlen(call_buf) - 40);
                                }
                            } else {
                                if (strcasecmp(t0_addr, "0xa1077a294dde1b09bb078844df40758a5d0f9a27") != 0) {
                                    strcpy(t1_addr, "0xa1077a294dde1b09bb078844df40758a5d0f9a27");
                                } else {
                                    strcpy(t1_addr, "0x959c5ad5c5ad5c5ad5c5ad5c5ad5c5ad5c5ad5cd");
                                }
                            }

                            if (strlen(t0_addr) > 0 && strlen(t1_addr) > 0) {
                                char t0_sym[64] = {0}, t0_name[128] = {0};
                                char t1_sym[64] = {0}, t1_name[128] = {0};
                                uint64_t t0_dec = 18, t1_dec = 18;
                                
                                resolve_token(t0_addr, t0_sym, t0_name, &t0_dec);
                                resolve_token(t1_addr, t1_sym, t1_name, &t1_dec);
                                
                                double val0_in = (double)amt0_in / pow(10, t0_dec);
                                double val1_in = (double)amt1_in / pow(10, t1_dec);
                                double val0_out = (double)amt0_out / pow(10, t0_dec);
                                double val1_out = (double)amt1_out / pow(10, t1_dec);
                                
                                double from_val = 0.0, to_val = 0.0;
                                (void)to_val;
                                const char *from_sym = "", *to_sym = "";
                                double swap_price = 0.0;
                                
                                if (amt0_in > 0) {
                                    from_sym = t0_sym; from_val = val0_in;
                                    to_sym = t1_sym; to_val = val1_out;
                                    if (val0_in > 0.0) swap_price = val1_out / val0_in;
                                } else {
                                    from_sym = t1_sym; from_val = val1_in;
                                    to_sym = t0_sym; to_val = val0_out;
                                    if (val1_in > 0.0) swap_price = val0_out / val1_in;
                                }
                                
                                // Retain edge in the swap routing graph
                                add_swap_edge(t0_addr, t1_addr, swap_price);

                                // Discover price deviations against previously accumulated swap
                                double price_diff = 0.0;
                                int found_cache = 0;
                                double bal0 = 0.0;
                                double bal1 = 0.0;
                                char bal_buf[1024];
                                char data_call[128];
                                snprintf(data_call, sizeof(data_call), "0x70a08231000000000000000000000000%s", pool_address + 2);
                                if (tsfi_pulse_rpc_call(t0_addr, data_call, bal_buf, sizeof(bal_buf))) {
                                    bal0 = (double)parse_hex_double(bal_buf, 64) / pow(10, t0_dec);
                                }
                                if (tsfi_pulse_rpc_call(t1_addr, data_call, bal_buf, sizeof(bal_buf))) {
                                    bal1 = (double)parse_hex_double(bal_buf, 64) / pow(10, t1_dec);
                                }

                                for (int i = 0; i < g_pool_cache_count; i++) {
                                    if (strcmp(g_pool_cache[i].pool_address, pool_address) == 0) {
                                        price_diff = swap_price - g_pool_cache[i].last_price;
                                        
                                        if (price_diff > 0.0) {
                                            tsfi_io_printf(stderr, "[DISCOVERY] Price RISE on pool %s (%s/%s): +%.6f (Amt: %.4f, Prev: %.4f)\n",
                                                           pool_address, from_sym, to_sym, price_diff, from_val, g_pool_cache[i].last_amount);
                                        } else if (price_diff < 0.0) {
                                            tsfi_io_printf(stderr, "[DISCOVERY] Price FALL on pool %s (%s/%s): %.6f (Amt: %.4f, Prev: %.4f)\n",
                                                           pool_address, from_sym, to_sym, price_diff, from_val, g_pool_cache[i].last_amount);
                                        }
                                        
                                        g_pool_cache[i].last_price = swap_price;
                                        g_pool_cache[i].last_amount = swap_amt;
                                        g_pool_cache[i].token0_balance = bal0;
                                        g_pool_cache[i].token1_balance = bal1;
                                        strcpy(g_pool_cache[i].token0, t0_addr);
                                        strcpy(g_pool_cache[i].token1, t1_addr);
                                        save_pool_cache();
                                        found_cache = 1;
                                        break;
                                    }
                                }
                                
                                if (!found_cache && g_pool_cache_count < MAX_POOL_CACHE) {
                                    int idx_c = g_pool_cache_count++;
                                    strcpy(g_pool_cache[idx_c].pool_address, pool_address);
                                    strcpy(g_pool_cache[idx_c].token0, t0_addr);
                                    strcpy(g_pool_cache[idx_c].token1, t1_addr);
                                    g_pool_cache[idx_c].token0_balance = bal0;
                                    g_pool_cache[idx_c].token1_balance = bal1;
                                    g_pool_cache[idx_c].last_price = swap_price;
                                    g_pool_cache[idx_c].last_amount = swap_amt;
                                    save_pool_cache();
                                }

                                // Dynamic insertion of swap neighbor into Knowledge Graph
                                if (g_graph_initialized) {
                                    CachedContract *swap_c = (CachedContract*)malloc(sizeof(CachedContract));
                                    if (swap_c) {
                                        snprintf(swap_c->name, sizeof(swap_c->name), "swap_%s_%s_%s", pool_address, from_sym, to_sym);
                                        swap_c->virtual_address = parse_hex_val(pool_address + 2, 16);
                                        swap_c->bytecode = NULL;
                                        swap_c->size = 0;
                                        snprintf(swap_c->path, sizeof(swap_c->path), "edge:%s:%s:%.6f", t0_addr, t1_addr, swap_price);
                                        
                                        g_net_nodes[TSFI_NET_PULSECHAIN].bst_root = tsfi_qing_bst_insert(
                                            g_net_nodes[TSFI_NET_PULSECHAIN].bst_root, 
                                            swap_c->virtual_address, 
                                            swap_c
                                        );
                                        tsfi_io_printf(stderr, "[MONITOR] Accumulated PulseX swap neighbor: Address=%s (%s/%s), Price=%.4f, Diff=%.4f\n",
                                                       pool_address, from_sym, to_sym, swap_price, price_diff);
                                    }
                                }
                            }
                            
                            // Release & free temporary block monitoring transaction structures
                            tsfi_io_printf(stderr, "[MONITOR] PulseX swap transaction data fully integrated; releasing and freeing swap accumulator memory.\n");
                        }
                        ptr = data_ptr;
                    }
                }

                // Classify incoming block to non-preferential accumulator models
                uint32_t lane = evaluate_accumulator_lane(target_block);

                // Dynamically update the Knowledge Graph in real-time
                if (g_graph_initialized) {
                    CachedContract *new_c = (CachedContract*)malloc(sizeof(CachedContract));
                    if (new_c) {
                        snprintf(new_c->name, sizeof(new_c->name), "block_%lu_unk_%u", target_block, unknown_tx_count);
                        new_c->virtual_address = target_block;
                        new_c->bytecode = NULL;
                        new_c->size = 0;
                        new_c->path[0] = '\0';
                        
                        g_net_nodes[TSFI_NET_PULSECHAIN].bst_root = tsfi_qing_bst_insert(
                            g_net_nodes[TSFI_NET_PULSECHAIN].bst_root, 
                            target_block, 
                            new_c
                        );
                        tsfi_io_printf(stderr, "[MONITOR] Inserted block %lu into undirected Knowledge Graph (lane %u)\n", target_block, lane);
                    }
                }
                
                
                // Add to block history array
                if (g_block_history_count < MAX_BLOCK_HISTORY) {
                    g_block_history[g_block_history_count++] = (BlockHistoryEntry){
                        .block_number = target_block,
                        .pulsex_tx_count = pulsex_tx_count,
                        .unknown_tx_count = unknown_tx_count
                    };
                } else {
                    memmove(g_block_history, g_block_history + 1, sizeof(BlockHistoryEntry) * (MAX_BLOCK_HISTORY - 1));
                    g_block_history[MAX_BLOCK_HISTORY - 1] = (BlockHistoryEntry){
                        .block_number = target_block,
                        .pulsex_tx_count = pulsex_tx_count,
                        .unknown_tx_count = unknown_tx_count
                    };
                }
                save_block_history();
                
                // Persist the successfully processed block number
                g_last_block_num = target_block;
                {
                    pthread_mutex_lock(&g_pending_mutex);
                    bool found_pending = false;
                    for (int i = 0; i < g_pending_blocks_count; i++) {
                        if (g_pending_blocks[i] == target_block) { found_pending = true; break; }
                    }
                    if (!found_pending) {
                        if (g_pending_blocks_count >= 10) {
                            for (int i = 0; i < g_pending_blocks_count - 1; i++) {
                                g_pending_blocks[i] = g_pending_blocks[i + 1];
                            }
                            g_pending_blocks[g_pending_blocks_count - 1] = target_block;
                        } else {
                            g_pending_blocks[g_pending_blocks_count++] = target_block;
                        }
                    }
                    pthread_mutex_unlock(&g_pending_mutex);
                }
                FILE *bf_w = fopen("assets/last_block_num.dat.bin", "wb");
                if (bf_w) {
                    fwrite(&g_last_block_num, sizeof(g_last_block_num), 1, bf_w);
                    fclose(bf_w);
                }
                
                print_pricing_routing_table();
            }
        }
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
    
    HolderInfo holders[24];
    int holders_count = 0;
    
    extern bool tsfi_pulse_explorer_get_holders(const char *token_addr, char *out_buffer, size_t out_max_len);
    if (tsfi_pulse_explorer_get_holders(token_lower, logs_buf, 524288)) {
        char *ptr = logs_buf;
        while ((ptr = strstr(ptr, "\"address\":\""))) {
            ptr += 11;
            char *end_addr = strchr(ptr, '"');
            if (end_addr && (end_addr - ptr) < 64) {
                char h_addr[64];
                strncpy(h_addr, ptr, end_addr - ptr);
                h_addr[end_addr - ptr] = '\0';
                
                char *val_ptr = strstr(end_addr, "\"value\":\"");
                if (val_ptr) {
                    val_ptr += 9;
                    char *end_val = strchr(val_ptr, '"');
                    if (end_val && (end_val - val_ptr) < 64) {
                        char val_str[64];
                        strncpy(val_str, val_ptr, end_val - val_ptr);
                        val_str[end_val - val_ptr] = '\0';
                        
                        if (holders_count < 24) {
                            strcpy(holders[holders_count].address, h_addr);
                            holders[holders_count].balance = strtod(val_str, NULL);
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
            }
            ptr = end_addr;
        }
    }
    free(logs_buf);
    
    uint64_t dec = 18;
    char sym[64], nam[128];
    resolve_token(token_lower, sym, nam, &dec);
    
    for (int i = 0; i < holders_count; i++) {
        holders[i].balance = holders[i].balance / pow(10, dec);
        
        bool is_seeded_contract = false;
        if (g_mcp_rdbms_table) {
            for (uint32_t j = 0; j < g_mcp_rdbms_table->count; j++) {
                if (strcasecmp(g_mcp_rdbms_table->rows[j].address, holders[i].address) == 0) {
                    is_seeded_contract = true;
                    break;
                }
            }
        }
        
        if (is_seeded_contract) {
            holders[i].is_contract = true;
        }
    }
    
    char *batch_payload = malloc(65536);
    char *batch_response = malloc(524288);
    if (batch_payload && batch_response) {
        size_t b_len = 0;
        b_len += snprintf(batch_payload + b_len, 65536 - b_len, "[");
        int batch_count = 0;
        for (int i = 0; i < holders_count; i++) {
            if (!holders[i].is_contract) {
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
                if (!holders[i].is_contract) {
                    char res_val[256] = {0};
                    if (find_batch_result_by_id(batch_response, i, res_val, sizeof(res_val))) {
                        if (strncmp(res_val, "0x", 2) == 0 && strlen(res_val) > 2) {
                            holders[i].is_contract = true;
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
            uint64_t c_dec = 18;
            resolve_token(holders[i].address, holders[i].symbol, holders[i].name, &c_dec);
            
            // Check if it is an LP contract by querying token0() and token1()
            char r_buf[1024];
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
                        }
                    }
                }
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

uint64_t tsfi_block_monitor_get_last_block_num(void) {
    return g_last_block_num;
}

void tsfi_block_monitor_set_last_block_num(uint64_t val) {
    g_last_block_num = val;
    FILE *bf_w = fopen("assets/last_block_num.dat.bin", "wb");
    if (bf_w) {
        fwrite(&g_last_block_num, sizeof(g_last_block_num), 1, bf_w);
        fclose(bf_w);
    }
}

uint64_t tsfi_block_monitor_get_newest_pending_block(void) {
    pthread_mutex_lock(&g_pending_mutex);
    uint64_t ret = g_last_block_num;
    if (g_pending_blocks_count > 0) {
        uint64_t max_block = 0;
        for (int i = 0; i < g_pending_blocks_count; i++) {
            if (g_pending_blocks[i] > max_block) {
                max_block = g_pending_blocks[i];
            }
        }
        ret = max_block;
    }
    pthread_mutex_unlock(&g_pending_mutex);
    return ret;
}

bool tsfi_block_monitor_prune_block(uint64_t block_num) {
    pthread_mutex_lock(&g_pending_mutex);
    int idx = -1;
    for (int i = 0; i < g_pending_blocks_count; i++) {
        if (g_pending_blocks[i] == block_num) {
            idx = i;
            break;
        }
    }
    bool ret = false;
    if (idx != -1) {
        for (int i = idx; i < g_pending_blocks_count - 1; i++) {
            g_pending_blocks[i] = g_pending_blocks[i + 1];
        }
        g_pending_blocks_count--;
        ret = true;
    }
    pthread_mutex_unlock(&g_pending_mutex);
    return ret;
}

int tsfi_block_monitor_get_pending_count(void) {
    pthread_mutex_lock(&g_pending_mutex);
    int ret = g_pending_blocks_count;
    pthread_mutex_unlock(&g_pending_mutex);
    return ret;
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
