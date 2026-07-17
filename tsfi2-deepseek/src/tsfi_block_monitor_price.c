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


SwapEdge g_swap_edges[MAX_SWAP_EDGES];
int g_swap_edges_count = 0;
static void save_swap_edges(void) {
    FILE *f = fopen("assets/swap_edges.dat.bin", "wb");
    if (f) {
        uint32_t count = (uint32_t)g_swap_edges_count;
        fwrite(&count, sizeof(count), 1, f);
        fwrite(g_swap_edges, sizeof(SwapEdge), count, f);
        fclose(f);
    }
}

void load_swap_edges(void) {
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
    if (strcasecmp(token_addr, wpls) == 0) {
        return 1.0;
    }
    
    extern bool tsfi_dexscreener_get_price(const char *token_addr, double *out_price_usd);
    double token_usd = 0.0;
    double wpls_usd = 0.0;
    if (tsfi_dexscreener_get_price(token_addr, &token_usd) &&
        tsfi_dexscreener_get_price(wpls, &wpls_usd)) {
        if (wpls_usd > 0.0) {
            return token_usd / wpls_usd;
        }
    }
    
    uint64_t dec = 18;
    char sym[64], nam[128];
    resolve_token(token_addr, sym, nam, &dec);
    
    char clean_token[64];
    const char *tok_ptr = token_addr;
    if (strncmp(tok_ptr, "0x", 2) == 0) tok_ptr += 2;
    strcpy(clean_token, tok_ptr);
    
    char clean_wpls[64];
    const char *wpls_ptr = wpls;
    if (strncmp(wpls_ptr, "0x", 2) == 0) wpls_ptr += 2;
    strcpy(clean_wpls, wpls_ptr);
    
    uint64_t val = 1;
    for (uint64_t i = 0; i < dec; i++) {
        val *= 10;
    }
    
    char calldata[512];
    snprintf(calldata, sizeof(calldata),
             "0xd06ca61f"
             "%064lx"
             "0000000000000000000000000000000000000000000000000000000000000040"
             "0000000000000000000000000000000000000000000000000000000000000002"
             "000000000000000000000000%.40s"
             "000000000000000000000000%.40s",
             val, clean_token, clean_wpls);
             
    char r_buf[2048];
    const char *router = "0x98bf93ebf5c380C0e6Ae8e192A7e2AE08edAcc02";
    if (tsfi_pulse_rpc_call(router, calldata, r_buf, sizeof(r_buf))) {
        const char *ptr = r_buf;
        if (strncmp(ptr, "0x", 2) == 0) ptr += 2;
        
        if (strlen(ptr) >= 256) {
            char val_hex[65];
            strncpy(val_hex, ptr + 192, 64);
            val_hex[64] = '\0';
            
            unsigned __int128 amount_out = 0;
            for (int k = 0; k < 64; k++) {
                char c = val_hex[k];
                amount_out *= 16;
                if (c >= '0' && c <= '9') amount_out += (c - '0');
                else if (c >= 'a' && c <= 'f') amount_out += (c - 'a' + 10);
                else if (c >= 'A' && c <= 'F') amount_out += (c - 'A' + 10);
            }
            double final_price = (double)(amount_out / 1000000000000000ULL);
            final_price /= 1000.0;
            return final_price;
        }
    }
    
    return bfs_price_route(token_addr, wpls);
}

PoolStateCache g_pool_cache[MAX_POOL_CACHE];
int g_pool_cache_count = 0;
void save_pool_cache(void) {
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

void load_pool_cache(void) {
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

