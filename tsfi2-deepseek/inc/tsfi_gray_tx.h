#ifndef TSFI_GRAY_TX_H
#define TSFI_GRAY_TX_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "tsfi_reuter_tx.h"

// 1. Jim Gray's Degrees of Consistency
typedef enum {
    CONSISTENCY_DEGREE_0, // Read Uncommitted (Dirty Read)
    CONSISTENCY_DEGREE_1, // Read Committed (Short Shared Lock)
    CONSISTENCY_DEGREE_2, // Repeatable Read (Long Shared Lock)
    CONSISTENCY_DEGREE_3  // Serializable (Long Shared + Gap Locking)
} tsfi_gray_consistency_degree;

// Lock request manager wrapper incorporating Consistency levels
int tsfi_gray_lock_read(tsfi_reuter_lock_head *lock_head, uint32_t tx_id, tsfi_gray_consistency_degree degree);
int tsfi_gray_lock_read_release(tsfi_reuter_lock_head *lock_head, uint32_t tx_id, tsfi_gray_consistency_degree degree);

// 2. Five-Minute Rule Cache Manager
typedef struct {
    uint32_t page_id;
    uint64_t last_access; // Unix timestamp or simulated clock
    bool active;
    tsfi_reuter_page page_content;
} tsfi_gray_cache_entry;

typedef struct {
    tsfi_gray_cache_entry entries[16];
    int size;
} tsfi_gray_cache_manager;

void tsfi_gray_cache_init(tsfi_gray_cache_manager *cm);
int tsfi_gray_cache_access(tsfi_gray_cache_manager *cm, uint32_t page_id, uint64_t current_time, tsfi_gray_cache_entry **entry_out);
int tsfi_gray_cache_evict_sweep(tsfi_gray_cache_manager *cm, uint64_t current_time, uint32_t *evicted_page_id);

// 3. Linear Hashing Temp Table Manager
typedef struct tsfi_gray_hash_node {
    uint32_t key;
    uint32_t val;
    struct tsfi_gray_hash_node *next;
} tsfi_gray_hash_node;

typedef struct {
    int bucket_count;
    int next_to_split;
    int max_buckets;
    int round_divisor;
    tsfi_gray_hash_node **buckets;
} tsfi_gray_linear_hash;

int tsfi_gray_lh_init(tsfi_gray_linear_hash *lh, int initial_buckets);
int tsfi_gray_lh_insert(tsfi_gray_linear_hash *lh, uint32_t key, uint32_t val);
int tsfi_gray_lh_lookup(const tsfi_gray_linear_hash *lh, uint32_t key, uint32_t *val_out);
void tsfi_gray_lh_destroy(tsfi_gray_linear_hash *lh);

#endif // TSFI_GRAY_TX_H
