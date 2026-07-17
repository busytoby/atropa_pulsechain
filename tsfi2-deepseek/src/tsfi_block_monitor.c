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



static uint64_t g_last_block_num = 0;
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



static double parse_hex_double(const char *hex, size_t len);
































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












typedef struct {
    uint64_t block_number;
    uint32_t pulsex_tx_count;
    uint32_t unknown_tx_count;
} BlockHistoryEntry;


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



void tsfi_block_monitor_init(void) {
    ensure_rdbms_table_initialized();
    load_discovered_tokens();
    load_contract_cache_table();
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
    add_discovered_token("0xefd766ccb38eaf1dfd701853bfce31359239f305", "DAI", "DAI from Ethereum", 18);
    add_discovered_token("0x15D3853B874d6A018eD6102B515Ef037Ad63e650", "USDC", "USDC from Ethereum", 6);
    add_discovered_token("0xC02aaA39b223FE8D0A0e5C4F27eAD9083C756Cc2", "WETH", "Wrapped Ether from Ethereum", 18);

    // Seed default swap edges so prices resolve immediately
    add_swap_edge("0x95acD3924370324F973352614Ea2624F0cf0d7eb", "0xA1077a294dDe1B09bB078844df40758a5D0f9a27", 0.0033);
    add_swap_edge("0xefd766ccb38eaf1dfd701853bfce31359239f305", "0xA1077a294dDe1B09bB078844df40758a5D0f9a27", 268.0587);
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
        strcpy(g_pool_cache[idx].pool_address, "0xe56043671df55de5cdf8459710433c10324de0ae");
        strcpy(g_pool_cache[idx].token0, "0xefd766ccb38eaf1dfd701853bfce31359239f305");
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



