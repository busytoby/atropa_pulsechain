#include "lau_yul_thunk_internal.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/time.h>

LauMemoNode *s_thunk_memo_bst_root = NULL;
pthread_mutex_t s_thunk_memo_bst_mutex = PTHREAD_MUTEX_INITIALIZER;
char s_execution_call_stack[16][64];
int s_execution_call_stack_depth = 0;
ContractDependency s_dependencies[256];
int s_dependency_count = 0;

void register_dependency(const char *parent, const char *child) {
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

LauMemoNode* bst_find_any(LauMemoNode *root, uint64_t hash) {
    while (root) {
        if (hash == root->signature_hash) return root;
        if (hash < root->signature_hash) {
            root = root->left;
        } else {
            root = root->right;
        }
    }
    return NULL;
}

LauMemoNode* bst_find(LauMemoNode *root, uint64_t hash) {
    while (root) {
        if (hash == root->signature_hash) {
            if (root->stale) return NULL;
            if (root->created_time_ms + root->ttl_ms < current_time_ms()) {
                return NULL;
            }
            return root;
        }
        if (hash < root->signature_hash) {
            root = root->left;
        } else {
            root = root->right;
        }
    }
    return NULL;
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

LauMemoNode* lau_yul_thunk_avl_insert(LauMemoNode *node, LauMemoNode *new_node) {
    if (!node) return new_node;
    if (new_node->signature_hash < node->signature_hash) {
        node->left = lau_yul_thunk_avl_insert(node->left, new_node);
    } else if (new_node->signature_hash > node->signature_hash) {
        node->right = lau_yul_thunk_avl_insert(node->right, new_node);
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

uint64_t current_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

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

bool g_cache_rehydrated = false;

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
                        s_thunk_memo_bst_root = lau_yul_thunk_avl_insert(s_thunk_memo_bst_root, new_node);
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

int count_active_nodes(LauMemoNode *node) {
    if (!node) return 0;
    return (node->stale ? 0 : 1) + count_active_nodes(node->left) + count_active_nodes(node->right);
}

void evict_lru_node(LauMemoNode *node, LauMemoNode **lru_found, uint64_t *oldest_time) {
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
