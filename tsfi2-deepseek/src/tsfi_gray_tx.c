#include "tsfi_gray_tx.h"
#include <string.h>
#include <stdlib.h>

// 1. Jim Gray's Degrees of Consistency Lock Management
int tsfi_gray_lock_read(tsfi_reuter_lock_head *lock_head, uint32_t tx_id, tsfi_gray_consistency_degree degree) {
    if (degree == CONSISTENCY_DEGREE_0) {
        return 0; // Degree 0 does not acquire read locks (dirty reads allowed)
    }
    // Degrees 1, 2, and 3 acquire Shared (S) locks
    return tsfi_reuter_lock_acquire(lock_head, tx_id, LOCK_MODE_S);
}

int tsfi_gray_lock_read_release(tsfi_reuter_lock_head *lock_head, uint32_t tx_id, tsfi_gray_consistency_degree degree) {
    if (degree == CONSISTENCY_DEGREE_0) {
        return 0; // No locks held
    }
    if (degree == CONSISTENCY_DEGREE_1) {
        // Degree 1 releases read locks immediately after reading (short lock)
        return tsfi_reuter_lock_release(lock_head, tx_id);
    }
    // Degrees 2 and 3 hold read locks until end of transaction (long locks)
    // Release is deferred to transaction completion (Rigorous 2PL release phase)
    return 0;
}

// 2. Five-Minute Rule Cache Manager
void tsfi_gray_cache_init(tsfi_gray_cache_manager *cm) {
    if (!cm) return;
    memset(cm, 0, sizeof(tsfi_gray_cache_manager));
}

int tsfi_gray_cache_access(tsfi_gray_cache_manager *cm, uint32_t page_id, uint64_t current_time, tsfi_gray_cache_entry **entry_out) {
    if (!cm || !entry_out) return -1;
    
    // Check if page already exists in cache
    for (int i = 0; i < cm->size; i++) {
        if (cm->entries[i].active && cm->entries[i].page_id == page_id) {
            cm->entries[i].last_access = current_time; // Update access timestamp
            *entry_out = &cm->entries[i];
            return 0; // Cache hit!
        }
    }
    
    // Load page into cache on cache miss
    for (int i = 0; i < 16; i++) {
        if (!cm->entries[i].active) {
            cm->entries[i].page_id = page_id;
            cm->entries[i].last_access = current_time;
            cm->entries[i].active = true;
            memset(&cm->entries[i].page_content, 0, sizeof(tsfi_reuter_page));
            cm->entries[i].page_content.page_id = page_id;
            
            if (i >= cm->size) {
                cm->size = i + 1;
            }
            
            *entry_out = &cm->entries[i];
            return 1; // Cache miss, loaded successfully
        }
    }
    
    return -2; // Cache full
}

int tsfi_gray_cache_evict_sweep(tsfi_gray_cache_manager *cm, uint64_t current_time, uint32_t *evicted_page_id) {
    if (!cm || !evicted_page_id) return -1;
    
    // Sweep cache entries and evict pages not accessed in the last 5 minutes (300 seconds)
    for (int i = 0; i < cm->size; i++) {
        if (cm->entries[i].active) {
            uint64_t idle_time = current_time > cm->entries[i].last_access ? 
                                 current_time - cm->entries[i].last_access : 0;
            if (idle_time >= 300) {
                *evicted_page_id = cm->entries[i].page_id;
                cm->entries[i].active = false;
                return 0; // Evicted successfully!
            }
        }
    }
    return 1; // No pages qualified for 5-minute rule eviction
}

// Helper: Hash function for linear hashing
static uint32_t lh_hash(uint32_t key, int count) {
    return key % count;
}

// 3. Linear Hashing Temp Table Manager
int tsfi_gray_lh_init(tsfi_gray_linear_hash *lh, int initial_buckets) {
    if (!lh || initial_buckets <= 0) return -1;
    
    lh->bucket_count = initial_buckets;
    lh->next_to_split = 0;
    lh->max_buckets = initial_buckets * 2;
    lh->round_divisor = initial_buckets;
    
    lh->buckets = (tsfi_gray_hash_node **)calloc(lh->max_buckets, sizeof(tsfi_gray_hash_node *));
    if (!lh->buckets) return -2;
    
    return 0;
}

int tsfi_gray_lh_insert(tsfi_gray_linear_hash *lh, uint32_t key, uint32_t val) {
    if (!lh || !lh->buckets) return -1;
    
    // Hash key using round divisor
    uint32_t bucket_idx = lh_hash(key, lh->round_divisor);
    if (bucket_idx < (uint32_t)lh->next_to_split) {
        // Recalculate hash with double round divisor for split buckets
        bucket_idx = lh_hash(key, lh->round_divisor * 2);
    }
    
    // Insert node into bucket list
    tsfi_gray_hash_node *new_node = (tsfi_gray_hash_node *)malloc(sizeof(tsfi_gray_hash_node));
    if (!new_node) return -2;
    new_node->key = key;
    new_node->val = val;
    new_node->next = lh->buckets[bucket_idx];
    lh->buckets[bucket_idx] = new_node;
    
    // Count bucket chain depth to trigger splitting
    int depth = 0;
    tsfi_gray_hash_node *curr = lh->buckets[bucket_idx];
    while (curr) {
        depth++;
        curr = curr->next;
    }
    
    // Split trigger: split next bucket if chain depth > 2 and room exists
    if (depth > 2 && (lh->bucket_count + 1) < lh->max_buckets) {
        int split_idx = lh->next_to_split;
        int new_bucket_idx = lh->bucket_count;
        
        tsfi_gray_hash_node *prev = NULL;
        curr = lh->buckets[split_idx];
        
        // Rehash split bucket chain using double round divisor
        while (curr) {
            uint32_t rehash_idx = lh_hash(curr->key, lh->round_divisor * 2);
            if (rehash_idx == (uint32_t)new_bucket_idx) {
                // Move node to new bucket
                tsfi_gray_hash_node *to_move = curr;
                if (prev) {
                    prev->next = curr->next;
                } else {
                    lh->buckets[split_idx] = curr->next;
                }
                curr = curr->next;
                
                to_move->next = lh->buckets[new_bucket_idx];
                lh->buckets[new_bucket_idx] = to_move;
                continue;
            }
            prev = curr;
            curr = curr->next;
        }
        
        lh->bucket_count++;
        lh->next_to_split++;
        if (lh->next_to_split == lh->round_divisor) {
            lh->next_to_split = 0; // Reset next split pointer
            lh->round_divisor *= 2; // Double round divisor
        }
    }
    
    return 0;
}

int tsfi_gray_lh_lookup(const tsfi_gray_linear_hash *lh, uint32_t key, uint32_t *val_out) {
    if (!lh || !lh->buckets || !val_out) return -1;
    
    uint32_t bucket_idx = lh_hash(key, lh->round_divisor);
    if (bucket_idx < (uint32_t)lh->next_to_split) {
        bucket_idx = lh_hash(key, lh->round_divisor * 2);
    }
    
    tsfi_gray_hash_node *curr = lh->buckets[bucket_idx];
    while (curr) {
        if (curr->key == key) {
            *val_out = curr->val;
            return 0; // Value found!
        }
        curr = curr->next;
    }
    return -2; // Value not found
}

void tsfi_gray_lh_destroy(tsfi_gray_linear_hash *lh) {
    if (!lh || !lh->buckets) return;
    
    for (int i = 0; i < lh->max_buckets; i++) {
        tsfi_gray_hash_node *curr = lh->buckets[i];
        while (curr) {
            tsfi_gray_hash_node *to_free = curr;
            curr = curr->next;
            free(to_free);
        }
    }
    free(lh->buckets);
    lh->buckets = NULL;
}
