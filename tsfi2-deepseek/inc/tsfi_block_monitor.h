#ifndef TSFI_BLOCK_MONITOR_H
#define TSFI_BLOCK_MONITOR_H

#include "tsfi_zmm_vm.h"
#include "tsfi_qing_graph.h"

typedef struct {
    uint64_t threshold;
    uint32_t left_child_idx;
    uint32_t right_child_idx;
    uint32_t branch_selector;
} InteropDecisionNode;

// Initialize Pulsechain block monitoring state
void tsfi_block_monitor_init(void);

// Poll for new Pulsechain blocks and drive VM/MQ inputs
void tsfi_block_monitor_tick(TsfiZmmVmState *state);

// Getter for the active undirected Knowledge Graph
tsfi_qing_graph_node* tsfi_block_monitor_get_graph(void);

// Query price of a token in PLS from retained swap pairs in Knowledge Graph
double tsfi_pulse_get_price_in_pls(const char *token_addr);

// Serialize all tracked contracts and their PLS prices to JSON
int tsfi_pulse_get_all_prices_json(char *out_buf, size_t max_len);

// Query and serialize all holders of a token sorted by balance
int tsfi_pulse_get_token_holders_json(const char *token_addr, char *out_buf, size_t max_len, bool force_refresh);

// Register a newly discovered token to the state mapping
void add_discovered_token(const char *addr, const char *symbol, const char *name, uint64_t decimals);

// Register a newly discovered swap edge rate mapping
void add_swap_edge(const char *t0, const char *t1, double price);

uint64_t tsfi_block_monitor_get_last_block_num(void);
void tsfi_block_monitor_set_last_block_num(uint64_t val);
uint64_t tsfi_block_monitor_get_newest_pending_block(void);
bool tsfi_block_monitor_prune_block(uint64_t block_num);
int tsfi_block_monitor_get_pending_count(void);
uint32_t tsfi_block_monitor_get_discovered_tokens_count(void);
void tsfi_block_monitor_get_discovered_token(uint32_t idx, char *out_addr, char *out_symbol, char *out_name);

#define MAX_DISCOVERED_TOKENS 512
#define MAX_CONTRACT_CACHE_ROWS 1024
#define MAX_POOL_CACHE 256
#define MAX_SWAP_EDGES 1024

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

typedef struct {
    char address[64];
    uint32_t is_contract; // 0=unknown, 1=wallet, 2=contract
    uint32_t is_lp;       // 0=unknown, 1=no, 2=yes
    char token0[64];
    char token0_sym[64];
    char token1[64];
    char token1_sym[64];
} LauContractCacheRow;

typedef struct {
    uint32_t active;
    uint32_t count;
    LauContractCacheRow rows[MAX_CONTRACT_CACHE_ROWS];
} LauContractCacheTable;

typedef struct ContractCacheBstNode {
    uint64_t key;
    uint32_t index;
    struct ContractCacheBstNode *left;
    struct ContractCacheBstNode *right;
} ContractCacheBstNode;

typedef struct {
    char pool_address[64];
    char token0[64];
    char token1[64];
    double token0_balance;
    double token1_balance;
    double last_price;
    double last_amount;
} PoolStateCache;

typedef struct {
    char token0[64];
    char token1[64];
    double price;
} SwapEdge;

typedef struct {
    const char *address;
    const char *symbol;
    const char *name;
    int decimals;
} HardcodedToken;

extern LauRdbmsTable *g_mcp_rdbms_table;
extern LauContractCacheTable *g_contract_cache_table;
extern PoolStateCache g_pool_cache[MAX_POOL_CACHE];
extern int g_pool_cache_count;
extern SwapEdge g_swap_edges[MAX_SWAP_EDGES];
extern int g_swap_edges_count;
extern const HardcodedToken g_known_tokens[10];

#define g_discovered_tokens (g_mcp_rdbms_table->rows)
#define g_discovered_tokens_count (g_mcp_rdbms_table->count)
#define KNOWN_TOKENS_COUNT (sizeof(g_known_tokens)/sizeof(g_known_tokens[0]))

void load_discovered_tokens(void);
void load_contract_cache_table(void);
void load_swap_edges(void);
void load_pool_cache(void);
void seed_tokens_from_addresses_sol(void);
void resolve_token(const char *address, char *out_symbol, char *out_name, uint64_t *out_decimals);
void save_pool_cache(void);
void save_discovered_tokens(void);
void save_contract_cache_table(void);
void ensure_rdbms_table_initialized(void);
void ensure_contract_cache_initialized(void);

#endif // TSFI_BLOCK_MONITOR_H
