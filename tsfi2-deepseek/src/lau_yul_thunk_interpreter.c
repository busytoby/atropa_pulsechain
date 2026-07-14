#include "lau_yul_thunk_internal.h"
#include "lau_memory.h"
#include "tsfi_qing_bst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

_Thread_local YulEvmContext g_yul_evm_context;

static uint64_t g_dyn_addr_counter = 0x9000000000000000ULL;

static pthread_mutex_t g_thunk_execute_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct LauMemoNode {
    uint64_t signature_hash;
    char contract_name[64];
    void *calldata_ptr;
    size_t calldatasize;
    void *retval_ptr;
    size_t retval_len;
    int height;
    uint64_t created_time_ms;
    uint64_t ttl_ms;
    bool stale;
    bool read_storage;
    uint64_t cache_hits;
    struct LauMemoNode *left;
    struct LauMemoNode *right;
} LauMemoNode;

static LauMemoNode *s_thunk_memo_bst_root = NULL;
static pthread_mutex_t s_thunk_memo_bst_mutex = PTHREAD_MUTEX_INITIALIZER;

static char s_execution_call_stack[16][64];
static int s_execution_call_stack_depth = 0;

typedef struct {
    char parent[64];
    char child[64];
} ContractDependency;

static ContractDependency s_dependencies[256];
static int s_dependency_count = 0;

static void register_dependency(const char *parent, const char *child) {
    for (int i = 0; i < s_dependency_count; i++) {
        if (strcmp(s_dependencies[i].parent, parent) == 0 && strcmp(s_dependencies[i].child, child) == 0) {
            return;
        }
    }
    if (s_dependency_count < 256) {
        strncpy(s_dependencies[s_dependency_count].parent, parent, 63);
        s_dependencies[s_dependency_count].parent[63] = '\0';
        strncpy(s_dependencies[s_dependency_count].child, child, 63);
        s_dependencies[s_dependency_count].child[63] = '\0';
        s_dependency_count++;
    }
}

static LauMemoNode* bst_find_any(LauMemoNode *root, uint64_t hash) {
    if (!root) return NULL;
    if (hash == root->signature_hash) return root;
    if (hash < root->signature_hash) return bst_find_any(root->left, hash);
    return bst_find_any(root->right, hash);
}

static LauMemoNode* bst_find(LauMemoNode *root, uint64_t hash) {
    if (!root) return NULL;
    if (hash == root->signature_hash) {
        extern uint64_t current_time_ms(void);
        if (root->stale) return NULL;
        if (root->created_time_ms + root->ttl_ms < current_time_ms()) {
            return NULL;
        }
        return root;
    }
    if (hash < root->signature_hash) return bst_find(root->left, hash);
    return bst_find(root->right, hash);
}

static int node_height(LauMemoNode *n) {
    return n ? n->height : 0;
}

static int max_val(int a, int b) {
    return a > b ? a : b;
}

static int get_balance(LauMemoNode *n) {
    return n ? node_height(n->left) - node_height(n->right) : 0;
}

static LauMemoNode* rotate_right(LauMemoNode *y) {
    LauMemoNode *x = y->left;
    LauMemoNode *T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max_val(node_height(y->left), node_height(y->right)) + 1;
    x->height = max_val(node_height(x->left), node_height(x->right)) + 1;
    return x;
}

static LauMemoNode* rotate_left(LauMemoNode *x) {
    LauMemoNode *y = x->right;
    LauMemoNode *T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max_val(node_height(x->left), node_height(x->right)) + 1;
    y->height = max_val(node_height(y->left), node_height(y->right)) + 1;
    return y;
}

static LauMemoNode* avl_insert(LauMemoNode *node, LauMemoNode *new_node) {
    if (!node) return new_node;
    if (new_node->signature_hash < node->signature_hash) {
        node->left = avl_insert(node->left, new_node);
    } else if (new_node->signature_hash > node->signature_hash) {
        node->right = avl_insert(node->right, new_node);
    } else {
        return node;
    }
    
    node->height = max_val(node_height(node->left), node_height(node->right)) + 1;
    int balance = get_balance(node);
    
    if (balance > 1 && new_node->signature_hash < node->left->signature_hash) {
        return rotate_right(node);
    }
    if (balance < -1 && new_node->signature_hash > node->right->signature_hash) {
        return rotate_left(node);
    }
    if (balance > 1 && new_node->signature_hash > node->left->signature_hash) {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    if (balance < -1 && new_node->signature_hash < node->right->signature_hash) {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }
    return node;
}

uint64_t bst_fnv1a_hash(const char *name, const uint8_t *calldata, size_t calldatasize) {
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; name[i] != '\0'; i++) {
        hash ^= (uint64_t)name[i];
        hash *= 1099511628211ULL;
    }
    for (size_t i = 0; i < calldatasize; i++) {
        hash ^= (uint64_t)calldata[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

static void bst_free_subtree(LauMemoNode *node) {
    if (!node) return;
    bst_free_subtree(node->left);
    bst_free_subtree(node->right);
    if (node->calldata_ptr) lau_free(node->calldata_ptr);
    if (node->retval_ptr) lau_free(node->retval_ptr);
    lau_free(node);
}

void lau_yul_thunk_cache_clear(void) {
    pthread_mutex_lock(&s_thunk_memo_bst_mutex);
    bst_free_subtree(s_thunk_memo_bst_root);
    s_thunk_memo_bst_root = NULL;
    pthread_mutex_unlock(&s_thunk_memo_bst_mutex);
}

int lau_yul_thunk_cache_height(void) {
    pthread_mutex_lock(&s_thunk_memo_bst_mutex);
    int h = s_thunk_memo_bst_root ? s_thunk_memo_bst_root->height : 0;
    pthread_mutex_unlock(&s_thunk_memo_bst_mutex);
    return h;
}

int lau_yul_thunk_cache_balance(void) {
    pthread_mutex_lock(&s_thunk_memo_bst_mutex);
    int balance = 0;
    if (s_thunk_memo_bst_root) {
        int lh = s_thunk_memo_bst_root->left ? s_thunk_memo_bst_root->left->height : 0;
        int rh = s_thunk_memo_bst_root->right ? s_thunk_memo_bst_root->right->height : 0;
        balance = lh - rh;
    }
    pthread_mutex_unlock(&s_thunk_memo_bst_mutex);
    return balance;
}

#include <sys/time.h>
uint64_t current_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

#include <dirent.h>

void write_thunk_to_disk(LauMemoNode *node) {
    char filepath[128];
    char temp_path[128];
    snprintf(filepath, sizeof(filepath), "assets/thunk_cache_%lu.json", node->signature_hash);
    snprintf(temp_path, sizeof(temp_path), "assets/temp_thunk_%lu.json", node->signature_hash);
    FILE *f = fopen(temp_path, "w");
    if (!f) {
        snprintf(filepath, sizeof(filepath), "../assets/thunk_cache_%lu.json", node->signature_hash);
        snprintf(temp_path, sizeof(temp_path), "../assets/temp_thunk_%lu.json", node->signature_hash);
        f = fopen(temp_path, "w");
    }
    if (f) {
        fprintf(f, "{\"signature_hash\": %lu, \"contract_name\": \"%s\", \"ttl_ms\": %lu, \"cache_hits\": %lu, \"calldata\": \"",
                node->signature_hash, node->contract_name, node->ttl_ms, node->cache_hits);
        for (size_t i = 0; i < node->calldatasize; i++) {
            fprintf(f, "%02x", ((uint8_t*)node->calldata_ptr)[i]);
        }
        fprintf(f, "\", \"retval\": \"");
        for (size_t i = 0; i < node->retval_len; i++) {
            fprintf(f, "%02x", ((uint8_t*)node->retval_ptr)[i]);
        }
        fprintf(f, "\"}\n");
        fclose(f);
        rename(temp_path, filepath);
    }
}

static uint8_t hex_to_byte(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return 0;
}

static void parse_hex_string(const char *hex, uint8_t *bytes, size_t *len) {
    size_t l = strlen(hex);
    *len = l / 2;
    for (size_t i = 0; i < *len; i++) {
        bytes[i] = (hex_to_byte(hex[2*i]) << 4) | hex_to_byte(hex[2*i+1]);
    }
}

static bool s_cache_rehydrated = false;

void lau_yul_thunk_cache_rehydrate(void) {
    const char *dir_path = "assets";
    DIR *d = opendir(dir_path);
    if (!d) {
        dir_path = "../assets";
        d = opendir(dir_path);
    }
    if (!d) return;

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (strncmp(dir->d_name, "thunk_cache_", 12) == 0 && strstr(dir->d_name, ".json")) {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, dir->d_name);
            FILE *f = fopen(filepath, "r");
            if (f) {
                char buf[4096];
                if (fgets(buf, sizeof(buf), f)) {
                    uint64_t sig_hash = 0;
                    char name[64] = "";
                    uint64_t ttl = 0;
                    uint64_t hits = 0;
                    char calldata_hex[1024] = "";
                    char retval_hex[1024] = "";
                    
                    char *p_sig = strstr(buf, "\"signature_hash\":");
                    char *p_name = strstr(buf, "\"contract_name\":");
                    char *p_ttl = strstr(buf, "\"ttl_ms\":");
                    char *p_hits = strstr(buf, "\"cache_hits\":");
                    char *p_call = strstr(buf, "\"calldata\":");
                    char *p_ret = strstr(buf, "\"retval\":");
                    
                    if (p_sig) sscanf(p_sig, "\"signature_hash\": %lu", &sig_hash);
                    if (p_name) sscanf(p_name, "\"contract_name\": \"%[^\"]\"", name);
                    if (p_ttl) sscanf(p_ttl, "\"ttl_ms\": %lu", &ttl);
                    if (p_hits) sscanf(p_hits, "\"cache_hits\": %lu", &hits);
                    if (p_call) sscanf(p_call, "\"calldata\": \"%[^\"]\"", calldata_hex);
                    if (p_ret) sscanf(p_ret, "\"retval\": \"%[^\"]\"", retval_hex);
                    
                    uint8_t calldata[512];
                    size_t calldatasize = 0;
                    parse_hex_string(calldata_hex, calldata, &calldatasize);
                    
                    uint8_t retval[512];
                    size_t retval_len = 0;
                    parse_hex_string(retval_hex, retval, &retval_len);
                    
                    extern uint64_t bst_fnv1a_hash(const char *n, const uint8_t *cd, size_t size);
                    uint64_t computed = bst_fnv1a_hash(name, calldata, calldatasize);
                    if (computed == sig_hash) {
                        LauMemoNode *new_node = (LauMemoNode*)lau_malloc(sizeof(LauMemoNode));
                        new_node->signature_hash = sig_hash;
                        strncpy(new_node->contract_name, name, 63);
                        new_node->contract_name[63] = '\0';
                        new_node->calldata_ptr = lau_malloc(calldatasize);
                        memcpy(new_node->calldata_ptr, calldata, calldatasize);
                        new_node->calldatasize = calldatasize;
                        if (retval_len > 0) {
                            new_node->retval_ptr = lau_malloc_wired(retval_len);
                            memcpy(new_node->retval_ptr, retval, retval_len);
                            new_node->retval_len = retval_len;
                            LauWiredHeader *h = (LauWiredHeader*)((char*)new_node->retval_ptr - 8192);
                            if (h) {
                                h->version = 1;
                                h->system_id = 42;
                                h->sealed = true;
                            }
                        } else {
                            new_node->retval_ptr = NULL;
                            new_node->retval_len = 0;
                        }
                        new_node->height = 1;
                        new_node->created_time_ms = current_time_ms();
                        new_node->ttl_ms = ttl;
                        new_node->stale = false;
                        new_node->cache_hits = hits;
                        new_node->left = NULL;
                        new_node->right = NULL;
                        
                        pthread_mutex_lock(&s_thunk_memo_bst_mutex);
                        extern LauMemoNode* avl_insert(LauMemoNode *root, LauMemoNode *node);
                        s_thunk_memo_bst_root = avl_insert(s_thunk_memo_bst_root, new_node);
                        pthread_mutex_unlock(&s_thunk_memo_bst_mutex);
                        printf("[QUADTREE] [REHYDRATE] Restored witness-validated cache node: hash=%lu, contract=%s\n", sig_hash, name);
                    }
                }
                fclose(f);
            }
        }
    }
    closedir(d);
}

static void bst_invalidate_contract(LauMemoNode *node, const char *name) {
    if (!node) return;
    if (strcmp(node->contract_name, name) == 0 && !node->stale) {
        node->stale = true;
        if (node->calldata_ptr) { lau_free(node->calldata_ptr); node->calldata_ptr = NULL; }
        if (node->retval_ptr) { lau_free(node->retval_ptr); node->retval_ptr = NULL; }
    }
    bst_invalidate_contract(node->left, name);
    bst_invalidate_contract(node->right, name);
}

static void bst_invalidate_storage_dependent(LauMemoNode *node) {
    if (!node) return;
    if (node->read_storage && !node->stale) {
        node->stale = true;
        if (node->calldata_ptr) { lau_free(node->calldata_ptr); node->calldata_ptr = NULL; }
        if (node->retval_ptr) { lau_free(node->retval_ptr); node->retval_ptr = NULL; }
    }
    bst_invalidate_storage_dependent(node->left);
    bst_invalidate_storage_dependent(node->right);
}

void lau_yul_thunk_cache_invalidate_storage(void) {
    pthread_mutex_lock(&s_thunk_memo_bst_mutex);
    bst_invalidate_storage_dependent(s_thunk_memo_bst_root);
    pthread_mutex_unlock(&s_thunk_memo_bst_mutex);
}

void lau_yul_thunk_cache_invalidate(const char *name) {
    pthread_mutex_lock(&s_thunk_memo_bst_mutex);
    bst_invalidate_contract(s_thunk_memo_bst_root, name);
    for (int i = 0; i < s_dependency_count; i++) {
        if (strcmp(s_dependencies[i].child, name) == 0) {
            bst_invalidate_contract(s_thunk_memo_bst_root, s_dependencies[i].parent);
        }
    }
    pthread_mutex_unlock(&s_thunk_memo_bst_mutex);
}

static int count_active_nodes(LauMemoNode *node) {
    if (!node) return 0;
    return (node->stale ? 0 : 1) + count_active_nodes(node->left) + count_active_nodes(node->right);
}

static void evict_lru_node(LauMemoNode *node, LauMemoNode **lru_found, uint64_t *oldest_time) {
    if (!node) return;
    if (!node->stale) {
        if (node->created_time_ms < *oldest_time) {
            *oldest_time = node->created_time_ms;
            *lru_found = node;
        }
    }
    evict_lru_node(node->left, lru_found, oldest_time);
    evict_lru_node(node->right, lru_found, oldest_time);
}

typedef struct {
    char name[64];
    uint64_t hits;
    uint64_t lookups;
} ContractStats;

static void accumulate_stats(LauMemoNode *node, ContractStats *arr, int *count, int max_contracts) {
    if (!node) return;
    int found_idx = -1;
    for (int i = 0; i < *count; i++) {
        if (strcmp(arr[i].name, node->contract_name) == 0) {
            found_idx = i;
            break;
        }
    }
    if (found_idx == -1 && *count < max_contracts) {
        found_idx = *count;
        strncpy(arr[found_idx].name, node->contract_name, 63);
        arr[found_idx].name[63] = '\0';
        arr[found_idx].hits = 0;
        arr[found_idx].lookups = 0;
        (*count)++;
    }
    if (found_idx != -1) {
        arr[found_idx].hits += node->cache_hits;
        arr[found_idx].lookups += (node->cache_hits + 1);
    }
    accumulate_stats(node->left, arr, count, max_contracts);
    accumulate_stats(node->right, arr, count, max_contracts);
}

int lau_yul_thunk_get_cache_stats(char *buf, size_t max_len) {
    ContractStats stats[32];
    int count = 0;
    pthread_mutex_lock(&s_thunk_memo_bst_mutex);
    accumulate_stats(s_thunk_memo_bst_root, stats, &count, 32);
    pthread_mutex_unlock(&s_thunk_memo_bst_mutex);

    size_t offset = 0;
    int n = snprintf(buf + offset, max_len - offset, "[");
    if (n > 0) offset += n;

    for (int i = 0; i < count; i++) {
        n = snprintf(buf + offset, max_len - offset, 
                     "{\"name\":\"%s\",\"hits\":%lu,\"lookups\":%lu}%s",
                     stats[i].name, stats[i].hits, stats[i].lookups,
                     (i == count - 1) ? "" : ",");
        if (n > 0) offset += n;
        if (offset >= max_len - 64) break;
    }

    snprintf(buf + offset, max_len - offset, "]");
    return count;
}

uint64_t g_thunk_cache_hits = 0;
uint64_t g_thunk_cache_lookups = 0;

typedef struct {
    uint64_t pc;
    uint8_t op;
} YulTraceStep;

#define TRACE_HISTORY_SIZE 32
YulTraceStep s_yul_trace_history[TRACE_HISTORY_SIZE];
uint32_t s_yul_trace_count = 0;
uint32_t s_yul_trace_head = 0;
pthread_mutex_t s_yul_trace_mutex = PTHREAD_MUTEX_INITIALIZER;

static bool execute_nested_call(YulEvmContext *ctx, uint64_t target_addr, uint64_t argsOffset, uint64_t argsSize, uint64_t retOffset, uint64_t retSize, u256_t *success_out);

static u256_t load_calldata_32(YulEvmContext *ctx, uint64_t offset) {
    u256_t r = {{0}};
    for (int i = 0; i < 32; i++) {
        uint64_t idx = offset + i;
        uint8_t byte = (idx < ctx->calldatasize) ? ctx->calldata[idx] : 0;
        r.d[3 - (i / 8)] |= ((uint64_t)byte) << (8 * (7 - (i % 8)));
    }
    return r;
}

static u256_t load_memory_32(YulEvmContext *ctx, uint64_t offset) {
    u256_t r = {{0}};
    if (offset + 32 > 524288) return r;
    for (int i = 0; i < 32; i++) {
        uint8_t byte = ctx->memory[offset + i];
        r.d[3 - (i / 8)] |= ((uint64_t)byte) << (8 * (7 - (i % 8)));
    }
    return r;
}

static void store_memory_32(YulEvmContext *ctx, uint64_t offset, u256_t val) {
    if (offset + 32 > 524288) return;
    for (int i = 0; i < 32; i++) {
        uint8_t byte = (uint8_t)(val.d[3 - (i / 8)] >> (8 * (7 - (i % 8))));
        ctx->memory[offset + i] = byte;
    }
}

static bool is_jumpdest(const uint8_t *bytecode, size_t size, uint64_t pc) {
    return pc < size && bytecode[pc] == 0x5b;
}

static void sync_raw_frame_to_yul_memory(uint8_t *memory) {
    static int raw_fd = -1;
    static uint8_t *raw_map = NULL;
    static bool raw_checked = false;
    if (!raw_checked) {
        raw_fd = open("/dev/shm/atropa_raw_frame.bin", O_RDONLY);
        if (raw_fd != -1) {
            struct stat st;
            if (fstat(raw_fd, &st) == 0 && st.st_size >= 256000) {
                raw_map = mmap(NULL, 256000, PROT_READ, MAP_SHARED, raw_fd, 0);
            }
        }
        raw_checked = true;
    }
    if (raw_map && raw_map != MAP_FAILED) {
        memcpy(memory + 131072, raw_map, 256000);
    }
}

// EVM execution loop
bool run_yul_bytecode(YulEvmContext *ctx, const uint8_t *bytecode, size_t size, const char *name) {
    (void)name;
    uint64_t pc = 0;
    ctx->reverted = false;
    ctx->stack_ptr = 0;
    sync_raw_frame_to_yul_memory(ctx->memory);

    while (pc < size) {
        uint8_t op = bytecode[pc];
        if (name && strcmp(name, "cpu6502") != 0) {
            pthread_mutex_lock(&s_yul_trace_mutex);
            s_yul_trace_history[s_yul_trace_head].pc = pc;
            s_yul_trace_history[s_yul_trace_head].op = op;
            s_yul_trace_head = (s_yul_trace_head + 1) % TRACE_HISTORY_SIZE;
            if (s_yul_trace_count < TRACE_HISTORY_SIZE) {
                s_yul_trace_count++;
            }
            pthread_mutex_unlock(&s_yul_trace_mutex);
        }
        if (op >= 0x60 && op <= 0x7f) { // PUSH1..PUSH32
            int push_bytes = op - 0x5f;
            u256_t val = {{0}};
            for (int i = 0; i < push_bytes; i++) {
                uint64_t idx = pc + 1 + i;
                uint8_t byte = (idx < size) ? bytecode[idx] : 0;
                val.d[3 - (i / 8)] |= ((uint64_t)byte) << (8 * (7 - (i % 8)));
            }
            // Right justify the value
            int shift_bits = (32 - push_bytes) * 8;
            val = u256_shr(val, shift_bits);
            
            if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at PUSH\n"); return false; }
            ctx->stack[ctx->stack_ptr++] = val;
            pc += 1 + push_bytes;
            continue;
        }

        if (op >= 0x80 && op <= 0x8f) { // DUP1..DUP16
            int dup_idx = op - 0x7f;
            if (ctx->stack_ptr < dup_idx) { printf("[DEBUG_EVM] Stack underflow at DUP\n"); return false; }
            if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at DUP\n"); return false; }
            ctx->stack[ctx->stack_ptr] = ctx->stack[ctx->stack_ptr - dup_idx];
            ctx->stack_ptr++;
            pc++;
            continue;
        }

        if (op >= 0x90 && op <= 0x9f) { // SWAP1..SWAP16
            int swap_idx = op - 0x8f;
            if (ctx->stack_ptr < swap_idx + 1) { printf("[DEBUG_EVM] Stack underflow at SWAP\n"); return false; }
            u256_t tmp = ctx->stack[ctx->stack_ptr - 1];
            ctx->stack[ctx->stack_ptr - 1] = ctx->stack[ctx->stack_ptr - 1 - swap_idx];
            ctx->stack[ctx->stack_ptr - 1 - swap_idx] = tmp;
            pc++;
            continue;
        }

        switch (op) {
            case 0x00: // STOP
                return true;
            case 0x01: { // ADD
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at ADD\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_add(a, b);
                break;
            }
            case 0x02: { // MUL
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at MUL\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_mul(a, b);
                break;
            }
            case 0x03: { // SUB
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SUB\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_sub(a, b);
                break;
            }
            case 0x04: { // DIV
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at DIV\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_div(a, b);
                break;
            }
            case 0x05: { // SDIV
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SDIV\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_sdiv(a, b);
                break;
            }
            case 0x06: { // MOD
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at MOD\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_mod(a, b);
                break;
            }
            case 0x07: { // SMOD
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SMOD\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_smod(a, b);
                break;
            }
            case 0x08: { // ADDMOD
                if (ctx->stack_ptr < 3) { printf("[DEBUG_EVM] Stack underflow at ADDMOD\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[--ctx->stack_ptr];
                u256_t N = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_addmod(a, b, N);
                break;
            }
            case 0x09: { // MULMOD
                if (ctx->stack_ptr < 3) { printf("[DEBUG_EVM] Stack underflow at MULMOD\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[--ctx->stack_ptr];
                u256_t N = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_mulmod(a, b, N);
                break;
            }
            case 0x0a: { // EXP
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at EXP\n"); return false; }
                u256_t base = ctx->stack[--ctx->stack_ptr];
                u256_t exponent = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_exp(base, exponent);
                break;
            }
            case 0x0b: { // SIGNEXTEND
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SIGNEXTEND\n"); return false; }
                u256_t k = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_signextend(val, k);
                break;
            }
            case 0x10: { // LT
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at LT\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = (u256_t){{ u256_lt(a, b), 0, 0, 0 }};
                break;
            }
            case 0x11: { // GT
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at GT\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = (u256_t){{ u256_gt(a, b), 0, 0, 0 }};
                break;
            }
            case 0x12: { // SLT
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SLT\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = (u256_t){{ (uint64_t)u256_slt(a, b), 0, 0, 0 }};
                break;
            }
            case 0x13: { // SGT
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SGT\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = (u256_t){{ (uint64_t)u256_sgt(a, b), 0, 0, 0 }};
                break;
            }
            case 0x14: { // EQ
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at EQ\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = (u256_t){{ u256_eq(a, b), 0, 0, 0 }};
                break;
            }
            case 0x15: { // ISZERO
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at ISZERO\n"); return false; }
                u256_t a = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = (u256_t){{ u256_iszero(a), 0, 0, 0 }};
                break;
            }
            case 0x16: { // AND
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at AND\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_and(a, b);
                break;
            }
            case 0x17: { // OR
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at OR\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_or(a, b);
                break;
            }
            case 0x18: { // XOR
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at XOR\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_xor(a, b);
                break;
            }
            case 0x19: { // NOT
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at NOT\n"); return false; }
                u256_t a = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_not(a);
                break;
            }
            case 0x1a: { // BYTE
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at BYTE\n"); return false; }
                u256_t pos = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[ctx->stack_ptr - 1];
                u256_t res = {{0}};
                if (pos.d[0] < 32) {
                    int byte_idx = 31 - pos.d[0];
                    uint8_t byte = (uint8_t)(val.d[byte_idx / 8] >> (8 * (byte_idx % 8)));
                    res.d[0] = byte;
                }
                ctx->stack[ctx->stack_ptr - 1] = res;
                break;
            }
            case 0x1b: { // SHL
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SHL\n"); return false; }
                u256_t shift = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_shl(val, shift.d[0]);
                break;
            }
            case 0x1c: { // SHR
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SHR\n"); return false; }
                u256_t shift = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_shr(val, shift.d[0]);
                break;
            }
            case 0x1d: { // SAR
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SAR\n"); return false; }
                u256_t shift = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[ctx->stack_ptr - 1];
                u256_t res = u256_shr(val, shift.d[0]);
                if (shift.d[0] < 256 && ((val.d[3] >> 63) & 1)) {
                    // Sign extend arithmetic shift right
                    for (int b = 255 - shift.d[0]; b < 256; b++) {
                        res.d[b / 64] |= (1ULL << (b % 64));
                    }
                } else if (shift.d[0] >= 256) {
                    res = ((val.d[3] >> 63) & 1) ? u256_not((u256_t){{0}}) : (u256_t){{0}};
                }
                ctx->stack[ctx->stack_ptr - 1] = res;
                break;
            }
            case 0x20: { // SHA3 / KECCAK256
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SHA3\n"); return false; }
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[ctx->stack_ptr - 1];
                u256_t res = {{0}};
                // Mock simple hash generator logic based on input bounds
                uint64_t hash = 5381;
                for (uint64_t i = 0; i < length.d[0]; i++) {
                    if (offset.d[0] + i < 524288) {
                        hash = ((hash << 5) + hash) + ctx->memory[offset.d[0] + i];
                    }
                }
                res.d[0] = hash;
                res.d[1] = hash ^ 0xDEADBEEF;
                res.d[2] = hash ^ 0xCAFEBABE;
                res.d[3] = hash ^ 0xBAADF00D;
                ctx->stack[ctx->stack_ptr - 1] = res;
                break;
            }
            case 0x30: { // ADDRESS
                u256_t r = {{0}};
                r.d[0] = ctx->self_address;
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at ADDRESS\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x32: { // ORIGIN
                u256_t r = {{ 0x4cc, 0, 0, 0 }};
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at ORIGIN\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x33: { // CALLER
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at CALLER\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = ctx->caller_address;
                break;
            }
            case 0x34: { // CALLVALUE
                u256_t r = {{0}}; // always 0 for thunks
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at CALLVALUE\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x35: { // CALLDATALOAD
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at CALLDATALOAD\n"); return false; }
                u256_t offset = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = load_calldata_32(ctx, offset.d[0]);
                break;
            }
            case 0x36: { // CALLDATASIZE
                u256_t r = {{0}};
                r.d[0] = ctx->calldatasize;
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at CALLDATASIZE\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x37: { // CALLDATACOPY
                if (ctx->stack_ptr < 3) { printf("[DEBUG_EVM] Stack underflow at CALLDATACOPY\n"); return false; }
                u256_t dest_offset = ctx->stack[--ctx->stack_ptr];
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                for (uint64_t i = 0; i < length.d[0]; i++) {
                    uint64_t src_idx = offset.d[0] + i;
                    uint64_t dest_idx = dest_offset.d[0] + i;
                    if (dest_idx < 524288) {
                        ctx->memory[dest_idx] = (src_idx < ctx->calldatasize) ? ctx->calldata[src_idx] : 0;
                    }
                }
                break;
            }
            case 0x38: { // CODESIZE
                u256_t r = {{size, 0, 0, 0}};
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at CODESIZE\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x39: { // CODECOPY
                if (ctx->stack_ptr < 3) { printf("[DEBUG_EVM] Stack underflow at CODECOPY\n"); return false; }
                u256_t destOffset = ctx->stack[--ctx->stack_ptr];
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t len = ctx->stack[--ctx->stack_ptr];
                
                uint64_t doff = destOffset.d[0];
                uint64_t src_off = offset.d[0];
                uint64_t l = len.d[0];
                
                for (uint64_t i = 0; i < l; i++) {
                    uint8_t val = 0;
                    if (src_off + i < size) {
                        val = bytecode[src_off + i];
                    }
                    if (doff + i < 524288) {
                        ctx->memory[doff + i] = val;
                    }
                }
                break;
            }
            case 0x3d: { // RETURNDATASIZE
                u256_t r = {{0}};
                r.d[0] = ctx->return_size;
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at RETURNDATASIZE\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x3e: { // RETURNDATACOPY
                if (ctx->stack_ptr < 3) { printf("[DEBUG_EVM] Stack underflow at RETURNDATACOPY\n"); return false; }
                u256_t dest_offset = ctx->stack[--ctx->stack_ptr];
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                for (uint64_t i = 0; i < length.d[0]; i++) {
                    uint64_t src_idx = offset.d[0] + i;
                    uint64_t dest_idx = dest_offset.d[0] + i;
                    if (dest_idx < 524288) {
                        ctx->memory[dest_idx] = (src_idx < ctx->return_size) ? ctx->return_data[src_idx] : 0;
                    }
                }
                break;
            }
            case 0x50: { // POP
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at POP\n"); return false; }
                ctx->stack_ptr--;
                break;
            }
            case 0x51: { // MLOAD
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at MLOAD\n"); return false; }
                u256_t offset = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = load_memory_32(ctx, offset.d[0]);
                break;
            }
            case 0x52: { // MSTORE
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at MSTORE\n"); return false; }
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[--ctx->stack_ptr];
                store_memory_32(ctx, offset.d[0], val);
                break;
            }
            case 0x53: { // MSTORE8
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at MSTORE8\n"); return false; }
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[--ctx->stack_ptr];
                if (offset.d[0] < 524288) {
                    ctx->memory[offset.d[0]] = (uint8_t)(val.d[0] & 0xFF);
                }
                break;
            }
            case 0x54: { // SLOAD
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at SLOAD\n"); return false; }
                u256_t key = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = context_sload(ctx, key);
                break;
            }
            case 0x55: { // SSTORE
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SSTORE\n"); return false; }
                u256_t key = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[--ctx->stack_ptr];
                context_sstore(ctx, key, val);
                extern void lau_yul_thunk_cache_invalidate(const char *name);
                lau_yul_thunk_cache_invalidate(name);
                break;
            }
            case 0x56: { // JUMP
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at JUMP\n"); return false; }
                u256_t dest = ctx->stack[--ctx->stack_ptr];
                if (!is_jumpdest(bytecode, size, dest.d[0])) {
                    printf("[DEBUG_EVM] Invalid JUMP destination: %lu\n", dest.d[0]);
                    return false;
                }
                pc = dest.d[0];
                continue;
            }
            case 0x57: { // JUMPI
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at JUMPI\n"); return false; }
                u256_t dest = ctx->stack[--ctx->stack_ptr];
                u256_t cond = ctx->stack[--ctx->stack_ptr];
                if (!u256_iszero(cond)) {
                    if (!is_jumpdest(bytecode, size, dest.d[0])) {
                        printf("[DEBUG_EVM] Invalid JUMPI destination: %lu\n", dest.d[0]);
                        return false;
                    }
                    pc = dest.d[0];
                    continue;
                }
                break;
            }
            case 0x5b: // JUMPDEST
                break;
            case 0x5f: { // PUSH0
                u256_t r = {{0}};
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at PUSH0\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0xf3: { // RETURN
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at RETURN\n"); return false; }
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                ctx->return_size = length.d[0] < 524288 ? length.d[0] : 524288;
                if (offset.d[0] < 524288) {
                     size_t avail = 524288 - offset.d[0];
                     if (ctx->return_size > avail) ctx->return_size = avail;
                     memcpy(ctx->return_data, ctx->memory + offset.d[0], ctx->return_size);
                } else {
                     ctx->return_size = 0;
                }
                return true;
            }
            case 0xfd: { // REVERT
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at REVERT\n"); return false; }
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                ctx->return_size = length.d[0] < 524288 ? length.d[0] : 524288;
                if (offset.d[0] < 524288) {
                     size_t avail = 524288 - offset.d[0];
                     if (ctx->return_size > avail) ctx->return_size = avail;
                     memcpy(ctx->return_data, ctx->memory + offset.d[0], ctx->return_size);
                } else {
                     ctx->return_size = 0;
                }
                printf("[DEBUG_EVM] REVERT triggered. Data (len %zu): ", ctx->return_size);
                for (size_t i = 0; i < (ctx->return_size < 64 ? ctx->return_size : 64); i++) {
                     printf("%02x", ctx->return_data[i]);
                }
                printf("\n");
                ctx->reverted = true;
                return false;
            }
            case 0xfe: { // INVALID
                printf("[DEBUG_EVM] Invalid opcode 0xfe executed\n");
                return false;
            }
            case 0xa0: // LOG0
            case 0xa1: // LOG1
            case 0xa2: // LOG2
            case 0xa3: // LOG3
            case 0xa4: { // LOG4
                int num_topics = op - 0xa0;
                if (ctx->stack_ptr < 2 + num_topics) { printf("[DEBUG_EVM] Stack underflow at LOG\n"); return false; }
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                if (ctx->log_count < 64) {
                    YulEventLog *log = &ctx->logs[ctx->log_count++];
                    log->address = ctx->self_address;
                    log->num_topics = num_topics;
                    for (int i = 0; i < num_topics; i++) {
                        log->topics[i] = ctx->stack[--ctx->stack_ptr];
                    }
                    log->data_size = length.d[0] < 2048 ? length.d[0] : 2048;
                    if (offset.d[0] < 524288) {
                        size_t avail = 524288 - offset.d[0];
                        if (log->data_size > avail) log->data_size = avail;
                        memcpy(log->data, ctx->memory + offset.d[0], log->data_size);
                    } else {
                        log->data_size = 0;
                    }
                } else {
                    // discard log, but pop stack topics
                    ctx->stack_ptr -= num_topics;
                }
                break;
            }
            case 0xf1: // CALL
            case 0xf2: // CALLCODE
            case 0xf4: // DELEGATECALL
            case 0xfa: { // STATICCALL
                bool is_delegate = (op == 0xf4);
                bool is_static = (op == 0xfa);
                int expected_stack = (is_delegate || is_static) ? 6 : 7;
                if (ctx->stack_ptr < expected_stack) { printf("[DEBUG_EVM] Stack underflow at CALL/DELEGATE/STATICCALL\n"); return false; }
                
                ctx->stack_ptr--; // discard gas limit
                u256_t target_addr_u256 = ctx->stack[--ctx->stack_ptr];
                uint64_t target_addr = target_addr_u256.d[0];
                
                if (!is_delegate && !is_static) {
                    ctx->stack_ptr--; // discard call value
                }
                
                u256_t argsOffset = ctx->stack[--ctx->stack_ptr];
                u256_t argsSize = ctx->stack[--ctx->stack_ptr];
                u256_t retOffset = ctx->stack[--ctx->stack_ptr];
                u256_t retSize = ctx->stack[--ctx->stack_ptr];
                
                u256_t success_out = {{0}};
                bool call_ok = execute_nested_call(ctx, target_addr, argsOffset.d[0], argsSize.d[0], retOffset.d[0], retSize.d[0], &success_out);
                (void)call_ok;
                ctx->stack[ctx->stack_ptr++] = success_out;
                break;
            }
            case 0x3b: { // EXTCODESIZE
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at EXTCODESIZE\n"); return false; }
                u256_t addr_val = ctx->stack[--ctx->stack_ptr];
                uint64_t search_addr = addr_val.d[0];
                size_t code_sz = 0;
                 tsfi_qing_bst_populate_safe();
                 CachedContract *c_match = tsfi_qing_bst_find_safe(search_addr);
                 if (c_match) {
                     code_sz = c_match->size;
                 }
                u256_t sz_val = {{ code_sz, 0, 0, 0 }};
                ctx->stack[ctx->stack_ptr++] = sz_val;
                break;
            }
            case 0x5a: { // GAS
                u256_t gas_val = {{ 1000000000ULL, 0, 0, 0 }};
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at GAS\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = gas_val;
                break;
            }
            case 0xf0: { // CREATE
                if (ctx->stack_ptr < 3) { printf("[DEBUG_EVM] Stack underflow at CREATE\n"); return false; }
                u256_t value = ctx->stack[--ctx->stack_ptr];
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                (void)value;

                uint8_t *initcode = NULL;
                size_t initcode_len = length.d[0];
                if (initcode_len > 0) {
                    initcode = malloc(initcode_len);
                    if (initcode) {
                        for (size_t i = 0; i < initcode_len; i++) {
                            uint64_t src = offset.d[0] + i;
                            initcode[i] = (src < 524288) ? ctx->memory[src] : 0;
                        }
                    }
                }

                uint64_t new_address = ++g_dyn_addr_counter;

                u256_t ret_addr = {{0}};
                if (initcode && initcode_len > 0) {
                    YulEvmContext *nested_ctx = calloc(1, sizeof(YulEvmContext));
                    if (nested_ctx) {
                        nested_ctx->self_address = new_address;
                        nested_ctx->caller_address.d[0] = ctx->self_address;
                        nested_ctx->storage_count = ctx->storage_count;
                        memcpy(nested_ctx->storage_keys, ctx->storage_keys, sizeof(ctx->storage_keys));
                        memcpy(nested_ctx->storage_vals, ctx->storage_vals, sizeof(ctx->storage_vals));
                        memcpy(nested_ctx->storage_addrs, ctx->storage_addrs, sizeof(ctx->storage_addrs));
                        nested_ctx->is_initcode = true;

                        bool success = run_yul_bytecode(nested_ctx, initcode, initcode_len, "dynamic");
                        if (success && !nested_ctx->reverted && nested_ctx->return_size > 0) {
                            if (g_cached_contracts_count < MAX_CACHED_CONTRACTS) {
                                CachedContract *c = &g_cached_contracts[g_cached_contracts_count++];
                                snprintf(c->name, sizeof(c->name), "dynamic_%lx", new_address);
                                c->bytecode = malloc(nested_ctx->return_size);
                                memcpy(c->bytecode, nested_ctx->return_data, nested_ctx->return_size);
                                c->size = nested_ctx->return_size;
                                c->virtual_address = new_address;
                                snprintf(c->path, sizeof(c->path), "dynamic");
                                ret_addr.d[0] = new_address;
                                tsfi_qing_bst_invalidate_safe();

                                for (int i = 0; i < nested_ctx->storage_count; i++) {
                                    u256_t raw_key = nested_ctx->storage_keys[i];
                                    bool found = false;
                                    for (int j = 0; j < ctx->storage_count; j++) {
                                        if (u256_eq(ctx->storage_keys[j], raw_key)) {
                                            ctx->storage_vals[j] = nested_ctx->storage_vals[i];
                                            ctx->storage_addrs[j] = nested_ctx->storage_addrs[i];
                                            found = true;
                                            break;
                                        }
                                    }
                                    if (!found && ctx->storage_count < 32768) {
                                        ctx->storage_keys[ctx->storage_count] = raw_key;
                                        ctx->storage_vals[ctx->storage_count] = nested_ctx->storage_vals[i];
                                        ctx->storage_addrs[ctx->storage_count] = nested_ctx->storage_addrs[i];
                                        ctx->storage_count++;
                                    }
                                }
                            }
                        }
                        free(nested_ctx);
                    }
                    free(initcode);
                }

                ctx->stack[ctx->stack_ptr++] = ret_addr;
                break;
            }
            case 0xf5: { // CREATE2
                if (ctx->stack_ptr < 4) { printf("[DEBUG_EVM] Stack underflow at CREATE2\n"); return false; }
                u256_t value = ctx->stack[--ctx->stack_ptr];
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                u256_t salt = ctx->stack[--ctx->stack_ptr];
                (void)value; (void)salt;

                uint8_t *initcode = NULL;
                size_t initcode_len = length.d[0];
                if (initcode_len > 0) {
                    initcode = malloc(initcode_len);
                    if (initcode) {
                        for (size_t i = 0; i < initcode_len; i++) {
                            uint64_t src = offset.d[0] + i;
                            initcode[i] = (src < 524288) ? ctx->memory[src] : 0;
                        }
                    }
                }

                uint64_t new_address = ++g_dyn_addr_counter;

                u256_t ret_addr = {{0}};
                if (initcode && initcode_len > 0) {
                    YulEvmContext *nested_ctx = calloc(1, sizeof(YulEvmContext));
                    if (nested_ctx) {
                        nested_ctx->self_address = new_address;
                        nested_ctx->caller_address.d[0] = ctx->self_address;
                        nested_ctx->storage_count = ctx->storage_count;
                        memcpy(nested_ctx->storage_keys, ctx->storage_keys, sizeof(ctx->storage_keys));
                        memcpy(nested_ctx->storage_vals, ctx->storage_vals, sizeof(ctx->storage_vals));
                        memcpy(nested_ctx->storage_addrs, ctx->storage_addrs, sizeof(ctx->storage_addrs));
                        nested_ctx->is_initcode = true;

                        bool success = run_yul_bytecode(nested_ctx, initcode, initcode_len, "dynamic");
                        if (success && !nested_ctx->reverted && nested_ctx->return_size > 0) {
                            if (g_cached_contracts_count < MAX_CACHED_CONTRACTS) {
                                CachedContract *c = &g_cached_contracts[g_cached_contracts_count++];
                                snprintf(c->name, sizeof(c->name), "dynamic_%lx", new_address);
                                c->bytecode = malloc(nested_ctx->return_size);
                                memcpy(c->bytecode, nested_ctx->return_data, nested_ctx->return_size);
                                c->size = nested_ctx->return_size;
                                c->virtual_address = new_address;
                                snprintf(c->path, sizeof(c->path), "dynamic");
                                ret_addr.d[0] = new_address;
                                tsfi_qing_bst_invalidate_safe();

                                for (int i = 0; i < nested_ctx->storage_count; i++) {
                                    u256_t raw_key = nested_ctx->storage_keys[i];
                                    bool found = false;
                                    for (int j = 0; j < ctx->storage_count; j++) {
                                        if (u256_eq(ctx->storage_keys[j], raw_key)) {
                                            ctx->storage_vals[j] = nested_ctx->storage_vals[i];
                                            ctx->storage_addrs[j] = nested_ctx->storage_addrs[i];
                                            found = true;
                                            break;
                                        }
                                    }
                                    if (!found && ctx->storage_count < 32768) {
                                        ctx->storage_keys[ctx->storage_count] = raw_key;
                                        ctx->storage_vals[ctx->storage_count] = nested_ctx->storage_vals[i];
                                        ctx->storage_addrs[ctx->storage_count] = nested_ctx->storage_addrs[i];
                                        ctx->storage_count++;
                                    }
                                }
                            }
                        }
                        free(nested_ctx);
                    }
                    free(initcode);
                }

                ctx->stack[ctx->stack_ptr++] = ret_addr;
                break;
            }
            default:
                printf("[DEBUG_EVM] Unsupported opcode: 0x%02X at PC %zu\n", op, pc);
                return false;
        }
        pc++;
    }
    return true;
}

bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t calldatasize, uint8_t *retval, size_t *retval_len) {
    // Passthrough to execute the full Yul VM state machine for WinchesterMQ to keep state compatibility with SCSI tests
    pthread_mutex_lock(&g_thunk_execute_mutex);
    CachedContract *c = NULL;
    uint64_t search_addr = 0;
    bool is_addr = false;
    if (name[0] == '0' && (name[1] == 'x' || name[1] == 'X')) {
        sscanf(name + 2, "%lx", &search_addr);
        is_addr = true;
    } else if (name[0] >= '0' && name[0] <= '9') {
        search_addr = strtoull(name, NULL, 0);
        is_addr = true;
    }

    tsfi_qing_bst_populate_safe();
    if (is_addr) {
        c = tsfi_qing_bst_find_safe(search_addr);
    } else {
        for (int i = 0; i < g_cached_contracts_count; i++) {
            if (strcmp(g_cached_contracts[i].name, name) == 0) {
                c = &g_cached_contracts[i];
                break;
            }
        }
    }

    if (!c) {
        printf("[YUL_THUNK] Error: Contract %s not initialized. Registered contracts count: %d\n", name, g_cached_contracts_count);
        for (int k = 0; k < g_cached_contracts_count; k++) {
            printf("  - %s (addr: 0x%lx)\n", g_cached_contracts[k].name, g_cached_contracts[k].virtual_address);
        }
        pthread_mutex_unlock(&g_thunk_execute_mutex);
        return false;
    }

    if (!s_cache_rehydrated) {
        s_cache_rehydrated = true;
        extern void lau_yul_thunk_cache_rehydrate(void);
        lau_yul_thunk_cache_rehydrate();
    }

    if (s_execution_call_stack_depth > 0 && s_execution_call_stack_depth < 16) {
        register_dependency(s_execution_call_stack[s_execution_call_stack_depth - 1], name);
    }
    if (s_execution_call_stack_depth < 16) {
        strncpy(s_execution_call_stack[s_execution_call_stack_depth++], name, 63);
    }

    uint64_t hash = bst_fnv1a_hash(name, calldata, calldatasize);

    bool skip_cache = (strcmp(name, "cpu6502") == 0 || strcmp(name, "WinchesterMQ") == 0);
    bool cache_hit = false;
    if (!skip_cache) {
        pthread_mutex_lock(&s_thunk_memo_bst_mutex);
        g_thunk_cache_lookups++;
        LauMemoNode *found = bst_find(s_thunk_memo_bst_root, hash);
        if (found &&
            strcmp(found->contract_name, name) == 0 &&
            found->calldatasize == calldatasize &&
            memcmp(found->calldata_ptr, calldata, calldatasize) == 0) {
            
            // Resolve wired header parameters from the allocated lau_memory payload
            bool meta_match = true;
            if (found->retval_ptr) {
                LauWiredHeader *h = (LauWiredHeader*)((char*)found->retval_ptr - 8192);
                if (h) {
                    // Filter: Require sealed results, version matching system parameters
                    if (!h->sealed || h->version != 1 || h->system_id != 42) {
                        meta_match = false;
                    }
                }
            }
            
            if (meta_match) {
                g_thunk_cache_hits++;
                found->cache_hits++;
                found->ttl_ms += 1000;
                if (found->ttl_ms > 30000) found->ttl_ms = 30000;
                found->created_time_ms = current_time_ms();
                if (retval && retval_len) {
                    size_t out_len = found->retval_len;
                    if (*retval_len < out_len) out_len = *retval_len;
                    memcpy(retval, found->retval_ptr, out_len);
                    *retval_len = out_len;
                }
                cache_hit = true;
            }
        }
        pthread_mutex_unlock(&s_thunk_memo_bst_mutex);
        if (cache_hit) {
            if (s_execution_call_stack_depth > 0) s_execution_call_stack_depth--;
            pthread_mutex_unlock(&g_thunk_execute_mutex);
            return true;
        }
    }

    extern bool g_storage_dirty;
    if (g_storage_dirty) {
        extern void persist_reconciliation_data(void);
        persist_reconciliation_data();
        g_storage_dirty = false;
    }
    g_transaction_diyat_tax_total = 0;
    g_yul_evm_context.log_count = 0;

    memset(g_yul_evm_context.calldata, 0, sizeof(g_yul_evm_context.calldata));
    size_t size_to_copy = calldatasize < sizeof(g_yul_evm_context.calldata) ? calldatasize : sizeof(g_yul_evm_context.calldata);
    memcpy(g_yul_evm_context.calldata, calldata, size_to_copy);
    g_yul_evm_context.calldatasize = size_to_copy;

    uint64_t prev_self_address = g_yul_evm_context.self_address;
    g_yul_evm_context.self_address = c->virtual_address;
    g_yul_evm_context.caller_address.d[0] = 0x4cc;
    g_yul_evm_context.caller_address.d[1] = 0;
    g_yul_evm_context.caller_address.d[2] = 0;
    g_yul_evm_context.caller_address.d[3] = 0;
    g_yul_evm_context.storage_read_occurred = false;
    bool success = run_yul_bytecode(&g_yul_evm_context, c->bytecode, c->size, name);

    if (success) {
        bool had_dirty_storage = g_storage_dirty;
        if (g_storage_dirty) {
            persist_reconciliation_data();
            g_storage_dirty = false;
            extern void lau_yul_thunk_cache_invalidate_storage(void);
            lau_yul_thunk_cache_invalidate_storage();
        }
        if (retval && retval_len) {
            size_t out_size = g_yul_evm_context.return_size < *retval_len ? g_yul_evm_context.return_size : *retval_len;
            memcpy(retval, g_yul_evm_context.return_data, out_size);
            *retval_len = out_size;
        }
        
        // Cache outputs for read-only dynamic thunk evaluations (skip if storage mutations occurred)
        if (!skip_cache && !had_dirty_storage && !g_yul_evm_context.reverted) {
            pthread_mutex_lock(&s_thunk_memo_bst_mutex);
            LauMemoNode *existing = bst_find_any(s_thunk_memo_bst_root, hash);
            if (existing) {
                existing->stale = false;
                existing->created_time_ms = current_time_ms();
                existing->read_storage = g_yul_evm_context.storage_read_occurred;
                if (existing->calldata_ptr) { lau_free(existing->calldata_ptr); existing->calldata_ptr = NULL; }
                existing->calldata_ptr = lau_malloc(calldatasize);
                memcpy(existing->calldata_ptr, calldata, calldatasize);
                existing->calldatasize = calldatasize;
                if (existing->retval_ptr) { lau_free(existing->retval_ptr); existing->retval_ptr = NULL; }
                if (retval && retval_len && *retval_len > 0) {
                    existing->retval_ptr = lau_malloc_wired(*retval_len);
                    memcpy(existing->retval_ptr, retval, *retval_len);
                    existing->retval_len = *retval_len;
                    LauWiredHeader *h = (LauWiredHeader*)((char*)existing->retval_ptr - 8192);
                    if (h) {
                        h->version = 1;
                        h->system_id = 42;
                        h->sealed = true;
                    }
                } else {
                    existing->retval_ptr = NULL;
                    existing->retval_len = 0;
                }
                extern void write_thunk_to_disk(LauMemoNode *node);
                write_thunk_to_disk(existing);
            } else {
                LauMemoNode *new_node = (LauMemoNode*)lau_malloc(sizeof(LauMemoNode));
                new_node->signature_hash = hash;
                strncpy(new_node->contract_name, name, 63);
                new_node->contract_name[63] = '\0';
                new_node->calldata_ptr = lau_malloc(calldatasize);
                memcpy(new_node->calldata_ptr, calldata, calldatasize);
                new_node->calldatasize = calldatasize;
                if (retval && retval_len && *retval_len > 0) {
                    new_node->retval_ptr = lau_malloc_wired(*retval_len);
                    memcpy(new_node->retval_ptr, retval, *retval_len);
                    new_node->retval_len = *retval_len;
                    
                    // Populate known LauWiredHeader data members to assist in search & selection
                    LauWiredHeader *h = (LauWiredHeader*)((char*)new_node->retval_ptr - 8192);
                    if (h) {
                        h->version = 1;
                        h->system_id = 42;
                        h->sealed = true;
                    }
                } else {
                    new_node->retval_ptr = NULL;
                    new_node->retval_len = 0;
                }
                new_node->height = 1;
                new_node->created_time_ms = current_time_ms();
                new_node->ttl_ms = 5000;
                new_node->stale = false;
                new_node->read_storage = g_yul_evm_context.storage_read_occurred;
                new_node->cache_hits = 0;
                new_node->left = NULL;
                new_node->right = NULL;
                
                if (count_active_nodes(s_thunk_memo_bst_root) >= 64) {
                    LauMemoNode *lru_node = NULL;
                    uint64_t oldest = -1ULL;
                    evict_lru_node(s_thunk_memo_bst_root, &lru_node, &oldest);
                    if (lru_node) {
                        lru_node->stale = true;
                        if (lru_node->calldata_ptr) { lau_free(lru_node->calldata_ptr); lru_node->calldata_ptr = NULL; }
                        if (lru_node->retval_ptr) { lau_free(lru_node->retval_ptr); lru_node->retval_ptr = NULL; }
                    }
                }
                s_thunk_memo_bst_root = avl_insert(s_thunk_memo_bst_root, new_node);
                extern void write_thunk_to_disk(LauMemoNode *node);
                write_thunk_to_disk(new_node);
            }
            pthread_mutex_unlock(&s_thunk_memo_bst_mutex);
        }
    } else {
        if (retval_len) *retval_len = 0;
    }
    g_yul_evm_context.self_address = prev_self_address;

    if (success && strcmp(name, "cpu6502") == 0) {
        /* Simulate FET discharge decay on registers A, X, Y (addresses 128, 129, 130) */
        uint64_t val_a = lau_yul_thunk_sload(128);
        uint64_t val_x = lau_yul_thunk_sload(129);
        uint64_t val_y = lau_yul_thunk_sload(130);

        /* Apply 2% FET discharge charge decay per CPU execution loop */
        uint64_t new_a = (uint64_t)(val_a * 0.98);
        uint64_t new_x = (uint64_t)(val_x * 0.98);
        uint64_t new_y = (uint64_t)(val_y * 0.98);

        lau_yul_thunk_sstore(128, new_a);
        lau_yul_thunk_sstore(129, new_x);
        lau_yul_thunk_sstore(130, new_y);
    }

    if (s_execution_call_stack_depth > 0) s_execution_call_stack_depth--;
    pthread_mutex_unlock(&g_thunk_execute_mutex);
    return success;
}

void lau_yul_thunk_sstore(uint64_t key, uint64_t value) {
    u256_t key_u256 = {{0}};
    key_u256.d[0] = key;
    u256_t val = {{0}};
    val.d[0] = value;
    uint64_t prev_addr = g_yul_evm_context.self_address;
    if (key >= 0xF000) {
        g_yul_evm_context.self_address = 0x200;
    }
    context_sstore(&g_yul_evm_context, key_u256, val);
    g_yul_evm_context.self_address = prev_addr;
    extern void lau_yul_thunk_cache_invalidate_storage(void);
    lau_yul_thunk_cache_invalidate_storage();
}

uint64_t lau_yul_thunk_sload(uint64_t key) {
    u256_t key_u256 = {{0}};
    key_u256.d[0] = key;
    uint64_t prev_addr = g_yul_evm_context.self_address;
    if (key >= 0xF000) {
        g_yul_evm_context.self_address = 0x200;
    }
    u256_t val = context_sload(&g_yul_evm_context, key_u256);
    g_yul_evm_context.self_address = prev_addr;
    return val.d[0];
}

bool execute_nested_call(YulEvmContext *ctx, uint64_t target_addr, uint64_t argsOffset, uint64_t argsSize, uint64_t retOffset, uint64_t retSize, u256_t *success_out) {
    success_out->d[0] = 1;

    uint32_t selector = 0;
    if (argsSize >= 4 && argsOffset < 524288) {
        selector = ((uint32_t)ctx->memory[argsOffset] << 24) |
                   ((uint32_t)ctx->memory[argsOffset + 1] << 16) |
                   ((uint32_t)ctx->memory[argsOffset + 2] << 8) |
                   ((uint32_t)ctx->memory[argsOffset + 3]);
    }

    char wmq_addr_str[128] = {0};
    FILE *wmq_file = fopen("../tmp/wmq_address.txt", "r");
    if (!wmq_file) wmq_file = fopen("tmp/wmq_address.txt", "r");
    if (!wmq_file) wmq_file = fopen("/home/mariarahel/src/tsfi2/atropa_pulsechain/tmp/wmq_address.txt", "r");
    if (wmq_file) {
        if (fgets(wmq_addr_str, sizeof(wmq_addr_str), wmq_file)) {
            size_t l = strlen(wmq_addr_str);
            while (l > 0 && (wmq_addr_str[l-1] == '\n' || wmq_addr_str[l-1] == '\r')) {
                wmq_addr_str[l-1] = '\0';
                l--;
            }
        }
        fclose(wmq_file);
    }
    if (strlen(wmq_addr_str) > 0) {
        uint64_t wmq_addr_u64 = parse_hex64(wmq_addr_str);
        if (target_addr == wmq_addr_u64) {
            printf("[EVM_INTERPRETER] Intercepted WinchesterMQ contract call at address 0x%lx with selector 0x%08x\n", target_addr, selector);
            if (argsSize > 0 && argsOffset < 524288) {
                bool is_io = false;
                if (selector == 0x98d400c0) {
                    is_io = true;
                } else if (argsSize >= 36 && selector == 0xccb077a0) {
                    char cmd_str[33] = {0};
                    size_t cmd_len = argsSize - 4;
                    if (cmd_len > 32) cmd_len = 32;
                    for (size_t i = 0; i < cmd_len; i++) {
                        cmd_str[i] = (char)ctx->memory[argsOffset + 4 + i];
                    }
                    if (strstr(cmd_str, "MM") || strstr(cmd_str, "MD") || strstr(cmd_str, "MU") ||
                        strstr(cmd_str, "MS") || strstr(cmd_str, "KD") || strstr(cmd_str, "KU") ||
                        strstr(cmd_str, "MO") || strstr(cmd_str, "MOUSE") || strstr(cmd_str, "KEY")) {
                        is_io = true;
                    }
                }
                
                if (is_io) {
                    printf("[EVM_INTERPRETER] WinchesterMQ keyboard/mouse event intercepted. Bypassing EVM RPC transaction.\n");
                } else {
                    char *data_hex = malloc(argsSize * 2 + 3);
                    if (data_hex) {
                        strcpy(data_hex, "0x");
                        for (size_t i = 0; i < argsSize; i++) {
                            sprintf(data_hex + 2 + i * 2, "%02x", ctx->memory[argsOffset + i]);
                        }
                        extern bool tsfi_pulse_rpc_send_wmq_transaction(const char *to_address, const char *data_hex);
                        bool rpc_ok = tsfi_pulse_rpc_send_wmq_transaction(wmq_addr_str, data_hex);
                        printf("[EVM_INTERPRETER] WinchesterMQ direct transaction sent: %s (status: %s)\n", data_hex, rpc_ok ? "SUCCESS" : "FAILED");
                        free(data_hex);
                    }
                }
            }
            success_out->d[0] = 1;
            return true;
        }
    }

    if (selector == 0x6872fc3c) { // Cho()
        u256_t mock_ret = {{ get_contract_address("cho"), 0, 0, 0 }};
        if (retOffset < 524288 && retSize > 0) {
            uint8_t temp_buf[32] = {0};
            write_u256_be(temp_buf, mock_ret);
            size_t src_offset = (retSize < 32) ? (32 - retSize) : 0;
            size_t copy_len = (retSize < 32) ? retSize : 32;
            for (size_t i = 0; i < copy_len; i++) {
                uint64_t dest = retOffset + i;
                if (dest < 524288) ctx->memory[dest] = temp_buf[src_offset + i];
            }
            ctx->return_size = retSize;
            memset(ctx->return_data, 0, sizeof(ctx->return_data));
            size_t to_copy = retSize > 524288 ? 524288 : retSize;
            memcpy(ctx->return_data, ctx->memory + retOffset, to_copy);
        }
        success_out->d[0] = 1;
        return true;
    }

    if (selector == 0xa5754463) { // Choa()
        u256_t mock_ret = {{ get_contract_address("choa"), 0, 0, 0 }};
        if (retOffset < 524288 && retSize > 0) {
            uint8_t temp_buf[32] = {0};
            write_u256_be(temp_buf, mock_ret);
            size_t src_offset = (retSize < 32) ? (32 - retSize) : 0;
            size_t copy_len = (retSize < 32) ? retSize : 32;
            for (size_t i = 0; i < copy_len; i++) {
                uint64_t dest = retOffset + i;
                if (dest < 524288) ctx->memory[dest] = temp_buf[src_offset + i];
            }
            ctx->return_size = retSize;
            memset(ctx->return_data, 0, sizeof(ctx->return_data));
            size_t to_copy = retSize > 524288 ? 524288 : retSize;
            memcpy(ctx->return_data, ctx->memory + retOffset, to_copy);
        }
        success_out->d[0] = 1;
        return true;
    }

    if (selector == 0x6fb7329c) { // Xiao()
        u256_t mock_ret = {{ get_contract_address("vmreq"), 0, 0, 0 }};
        if (retOffset < 524288 && retSize > 0) {
            uint8_t temp_buf[32] = {0};
            write_u256_be(temp_buf, mock_ret);
            size_t src_offset = (retSize < 32) ? (32 - retSize) : 0;
            size_t copy_len = (retSize < 32) ? retSize : 32;
            for (size_t i = 0; i < copy_len; i++) {
                uint64_t dest = retOffset + i;
                if (dest < 524288) ctx->memory[dest] = temp_buf[src_offset + i];
            }
            ctx->return_size = retSize;
            memset(ctx->return_data, 0, sizeof(ctx->return_data));
            size_t to_copy = retSize > 524288 ? 524288 : retSize;
            memcpy(ctx->return_data, ctx->memory + retOffset, to_copy);
        }
        success_out->d[0] = 1;
        return true;
    }

    if (selector == 0x604a6fa9) { // Random()
        uint64_t vmreq_addr = get_contract_address("vmreq");
        if (target_addr != vmreq_addr) {
            uint64_t rand_val = 50000 + (rand() % 50000);
            u256_t mock_ret = {{ rand_val, 0, 0, 0 }};
            if (retOffset < 524288 && retSize > 0) {
                uint8_t temp_buf[32] = {0};
                write_u256_be(temp_buf, mock_ret);
                size_t src_offset = (retSize < 32) ? (32 - retSize) : 0;
                size_t copy_len = (retSize < 32) ? retSize : 32;
                for (size_t i = 0; i < copy_len; i++) {
                    uint64_t dest = retOffset + i;
                    if (dest < 524288) ctx->memory[dest] = temp_buf[src_offset + i];
                }
                ctx->return_size = retSize;
                memset(ctx->return_data, 0, sizeof(ctx->return_data));
                size_t to_copy = retSize > 524288 ? 524288 : retSize;
                memcpy(ctx->return_data, ctx->memory + retOffset, to_copy);
            }
            success_out->d[0] = 1;
            return true;
        }
    }

    if (selector == 0xd9270a5a) { // GetLibraryAddress(string)
        bool is_strings = false;
        bool is_attribute = false;
        bool is_reactions = false;
        for (uint64_t idx = 4; idx + 7 <= argsSize; idx++) {
            if (memcmp(&ctx->memory[argsOffset + idx], "strings", 7) == 0) is_strings = true;
            if (memcmp(&ctx->memory[argsOffset + idx], "attribute", 9) == 0) is_attribute = true;
            if (memcmp(&ctx->memory[argsOffset + idx], "reactions", 9) == 0) is_reactions = true;
        }
        uint64_t lib_addr = 0;
        if (is_strings) {
            lib_addr = get_contract_address("strings");
        } else if (is_attribute) {
            lib_addr = get_contract_address("libattribute");
        } else if (is_reactions) {
            lib_addr = get_contract_address("corereactions");
        }
        if (lib_addr != 0) {
            u256_t mock_ret = {{ lib_addr, 0, 0, 0 }};
            if (retOffset < 524288 && retSize > 0) {
                uint8_t temp_buf[32] = {0};
                write_u256_be(temp_buf, mock_ret);
                size_t src_offset = (retSize < 32) ? (32 - retSize) : 0;
                size_t copy_len = (retSize < 32) ? retSize : 32;
                for (size_t i = 0; i < copy_len; i++) {
                    uint64_t dest = retOffset + i;
                    if (dest < 524288) ctx->memory[dest] = temp_buf[src_offset + i];
                }
                ctx->return_size = retSize;
                memset(ctx->return_data, 0, sizeof(ctx->return_data));
                size_t to_copy = retSize > 524288 ? 524288 : retSize;
                memcpy(ctx->return_data, ctx->memory + retOffset, to_copy);
            }
            success_out->d[0] = 1;
            return true;
        }
    }

    if (target_addr == 0x05) { // BigModExp
        uint8_t temp_calldata[192] = {0};
        uint64_t copy_calldata_size = argsSize < 192 ? argsSize : 192;
        for (uint64_t i = 0; i < copy_calldata_size; i++) {
            uint64_t src = argsOffset + i;
            temp_calldata[i] = (src < 524288) ? ctx->memory[src] : 0;
        }

        u256_t base = read_u256_be(temp_calldata + 96);
        u256_t exp = read_u256_be(temp_calldata + 128);
        u256_t mod = read_u256_be(temp_calldata + 160);
        u256_t res = u256_mod_pow(base, exp, mod);
        printf("[DEBUG_MATH] BigModExp: base=%lu exp=%lu mod=%lu res=%lu\n", base.d[0], exp.d[0], mod.d[0], res.d[0]);

        uint8_t res_bytes[32];
        write_u256_be(res_bytes, res);

        uint64_t copy_size = retSize < 32 ? retSize : 32;
        for (uint64_t i = 0; i < copy_size; i++) {
            uint64_t dest = retOffset + i;
            if (dest < 524288) {
                ctx->memory[dest] = res_bytes[i];
            }
        }
        ctx->return_size = 32;
        memcpy(ctx->return_data, res_bytes, 32);
        success_out->d[0] = 1;
        return true;
    }

    tsfi_qing_bst_populate_safe();
    CachedContract *target = tsfi_qing_bst_find_safe(target_addr);
    if (!target) {
        success_out->d[0] = 1;
        u256_t mock_ret = {{ 0x1000, 0, 0, 0 }};
        if (retOffset < 524288 && retSize > 0) {
            uint8_t temp_buf[32] = {0};
            write_u256_be(temp_buf, mock_ret);
            size_t src_offset = (retSize < 32) ? (32 - retSize) : 0;
            size_t copy_len = (retSize < 32) ? retSize : 32;
            for (size_t i = 0; i < copy_len; i++) {
                uint64_t dest = retOffset + i;
                if (dest < 524288) ctx->memory[dest] = temp_buf[src_offset + i];
            }
            ctx->return_size = retSize;
            memset(ctx->return_data, 0, sizeof(ctx->return_data));
            size_t to_copy = retSize > 524288 ? 524288 : retSize;
            memcpy(ctx->return_data, ctx->memory + retOffset, to_copy);
        }
        return true;
    }
    
    YulEvmContext *nested_ctx = calloc(1, sizeof(YulEvmContext));
    if (nested_ctx) {
        nested_ctx->calldatasize = (argsSize > 65536) ? 65536 : argsSize;
        for (uint64_t i = 0; i < nested_ctx->calldatasize; i++) {
            uint64_t src = argsOffset + i;
            nested_ctx->calldata[i] = (src < 524288) ? ctx->memory[src] : 0;
        }
        
        nested_ctx->storage_count = ctx->storage_count;
        memcpy(nested_ctx->storage_keys, ctx->storage_keys, sizeof(ctx->storage_keys));
        memcpy(nested_ctx->storage_vals, ctx->storage_vals, sizeof(ctx->storage_vals));
        nested_ctx->self_address = target_addr;

        if ((strcmp(target->name, "diyat") == 0 || target_addr == 858021) && nested_ctx->calldatasize >= 4) {
            uint32_t sel = ((uint32_t)nested_ctx->calldata[0] << 24) |
                            ((uint32_t)nested_ctx->calldata[1] << 16) |
                            ((uint32_t)nested_ctx->calldata[2] << 8)  |
                            ((uint32_t)nested_ctx->calldata[3]);
            if (sel == 0x904a4bc3 && nested_ctx->calldatasize >= 68) {
                uint64_t taxAmount = 0;
                for (int i = 0; i < 8; i++) {
                    taxAmount = (taxAmount << 8) | nested_ctx->calldata[60 + i];
                }
                g_transaction_diyat_tax_total += taxAmount;
            }
        }
        nested_ctx->caller_address.d[0] = ctx->self_address;
        nested_ctx->caller_address.d[1] = 0;
        nested_ctx->caller_address.d[2] = 0;
        nested_ctx->caller_address.d[3] = 0;
        
        bool run_ok = run_yul_bytecode(nested_ctx, target->bytecode, target->size, target->name);
        
        if (run_ok && !nested_ctx->reverted) {
            uint64_t copy_size = retSize;
            if (copy_size > nested_ctx->return_size) {
                copy_size = nested_ctx->return_size;
            }
            for (uint64_t i = 0; i < copy_size; i++) {
                uint64_t dest = retOffset + i;
                if (dest < 524288) {
                    ctx->memory[dest] = nested_ctx->return_data[i];
                }
            }
            
            for (int i = 0; i < nested_ctx->storage_count; i++) {
                u256_t raw_key = nested_ctx->storage_keys[i];
                bool found = false;
                for (int j = 0; j < ctx->storage_count; j++) {
                    if (u256_eq(ctx->storage_keys[j], raw_key)) {
                        ctx->storage_vals[j] = nested_ctx->storage_vals[i];
                        ctx->storage_addrs[j] = nested_ctx->storage_addrs[i];
                        found = true;
                        break;
                    }
                }
                if (!found && ctx->storage_count < 32768) {
                    ctx->storage_keys[ctx->storage_count] = raw_key;
                    ctx->storage_vals[ctx->storage_count] = nested_ctx->storage_vals[i];
                    ctx->storage_addrs[ctx->storage_count] = nested_ctx->storage_addrs[i];
                    ctx->storage_count++;
                }
            }
            
            ctx->return_size = nested_ctx->return_size;
            if (ctx->return_size > 524288) ctx->return_size = 524288;
            memcpy(ctx->return_data, nested_ctx->return_data, ctx->return_size);
        } else {
            printf("[DEBUG_EVM] Nested call to %s reverted/failed. run_ok=%d reverted=%d return_size=%zu\n", target->name, run_ok, nested_ctx->reverted, nested_ctx->return_size);
            if (nested_ctx->return_size > 0) {
                printf("  Revert data hex: ");
                for (size_t i = 0; i < nested_ctx->return_size && i < 128; i++) {
                    printf("%02x", nested_ctx->return_data[i]);
                }
                printf("\n");
            }
            success_out->d[0] = 0;
            if (retOffset < 524288 && retSize > 0 && nested_ctx->return_size > 0) {
                size_t copy_size = retSize < nested_ctx->return_size ? retSize : nested_ctx->return_size;
                for (size_t i = 0; i < copy_size; i++) {
                    uint64_t dest = retOffset + i;
                    if (dest < 524288) ctx->memory[dest] = nested_ctx->return_data[i];
                }
                ctx->return_size = nested_ctx->return_size;
                memcpy(ctx->return_data, nested_ctx->return_data, ctx->return_size);
            } else {
                ctx->return_size = 0;
            }
        }
        free(nested_ctx);
    } else {
        success_out->d[0] = 0;
    }
    return true;
}
