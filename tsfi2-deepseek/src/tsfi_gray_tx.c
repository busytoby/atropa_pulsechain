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

// 4. Fault-Tolerant Process Pairs
int tsfi_gray_pp_checkpoint_sync(tsfi_gray_process_pair *pair, uint32_t tx_id, uint64_t lsn, const tsfi_reuter_page *page) {
    if (!pair) return -1;
    
    pair->active_tx_id = tx_id;
    pair->last_sync_lsn = lsn;
    if (page) {
        pair->replicated_page = *page;
    }
    return 0;
}

int tsfi_gray_pp_failover(tsfi_gray_process_pair *pair, tsfi_reuter_page *active_page) {
    if (!pair || !active_page) return -1;
    
    pair->primary_failed = true;
    *active_page = pair->replicated_page; // Restore page state on failover
    return 0;
}

// 5. Debit-Credit Workload (TPC-A Simulator)
int tsfi_gray_tpca_exec(tsfi_gray_tpca_state *state, int32_t delta, int *out_history_val) {
    if (!state || !out_history_val) return -1;
    
    // Standard Debit-Credit logic:
    // Update Branch, Teller, and Account balances
    state->branch_balance += delta;
    state->teller_balance += delta;
    state->account_balance += delta;
    
    // Generate history record value
    *out_history_val = state->account_balance + state->teller_balance + state->branch_balance;
    return 0;
}

// 6. Cascading Abort Chain Tracker
void tsfi_gray_abort_tracker_init(tsfi_gray_abort_tracker *at) {
    if (!at) return;
    memset(at, 0, sizeof(tsfi_gray_abort_tracker));
}

int tsfi_gray_abort_add_dep(tsfi_gray_abort_tracker *at, uint32_t writer_id, uint32_t reader_id) {
    if (!at) return -1;
    if (at->count >= 16) return -2; // Limit exceeded
    
    at->dependencies[at->count].writer_tx_id = writer_id;
    at->dependencies[at->count].reader_tx_id = reader_id;
    at->dependencies[at->count].active = true;
    at->count++;
    return 0;
}

int tsfi_gray_abort_cascade(tsfi_gray_abort_tracker *at, uint32_t aborted_tx_id, uint32_t *cascaded_aborts_out, int *cascade_count) {
    if (!at || !cascaded_aborts_out || !cascade_count) return -1;
    
    // Find all active readers that read from this aborted writer transaction
    for (int i = 0; i < at->count; i++) {
        if (at->dependencies[i].active && at->dependencies[i].writer_tx_id == aborted_tx_id) {
            uint32_t dependent_tx = at->dependencies[i].reader_tx_id;
            
            // Check if already recorded
            bool already_recorded = false;
            for (int j = 0; j < *cascade_count; j++) {
                if (cascaded_aborts_out[j] == dependent_tx) {
                    already_recorded = true;
                    break;
                }
            }
            
            if (!already_recorded) {
                cascaded_aborts_out[*cascade_count] = dependent_tx;
                (*cascade_count)++;
                at->dependencies[i].active = false;
                
                // Recursively cascade aborts downstream
                tsfi_gray_abort_cascade(at, dependent_tx, cascaded_aborts_out, cascade_count);
            }
        }
    }
    return 0;
}

// 7. OLAP "Data Cube" Aggregation Operator
int tsfi_gray_cube_aggregate(const tsfi_gray_cube_entry *raw_data, int data_count, 
                             tsfi_gray_cube_entry *cube_out, int *cube_count_out) {
    if (!raw_data || data_count <= 0 || !cube_out || !cube_count_out) return -1;
    
    int count = 0;
    
    // 1. Calculate leaf-level combinations (Branch + Teller)
    for (int i = 0; i < data_count; i++) {
        bool found = false;
        for (int j = 0; j < count; j++) {
            if (cube_out[j].branch_id == raw_data[i].branch_id && 
                cube_out[j].teller_id == raw_data[i].teller_id) {
                cube_out[j].total_amount += raw_data[i].total_amount;
                found = true;
                break;
            }
        }
        if (!found) {
            cube_out[count].branch_id = raw_data[i].branch_id;
            cube_out[count].teller_id = raw_data[i].teller_id;
            cube_out[count].total_amount = raw_data[i].total_amount;
            count++;
        }
    }
    
    // 2. Calculate sub-totals for each Branch (Branch, ALL tellers: teller_id = 0xFFFFFFFF)
    for (int i = 0; i < data_count; i++) {
        bool found = false;
        for (int j = 0; j < count; j++) {
            if (cube_out[j].branch_id == raw_data[i].branch_id && 
                cube_out[j].teller_id == 0xFFFFFFFF) {
                cube_out[j].total_amount += raw_data[i].total_amount;
                found = true;
                break;
            }
        }
        if (!found) {
            cube_out[count].branch_id = raw_data[i].branch_id;
            cube_out[count].teller_id = 0xFFFFFFFF;
            cube_out[count].total_amount = raw_data[i].total_amount;
            count++;
        }
    }
    
    // 3. Calculate sub-totals for each Teller (ALL branches, Teller: branch_id = 0xFFFFFFFF)
    for (int i = 0; i < data_count; i++) {
        bool found = false;
        for (int j = 0; j < count; j++) {
            if (cube_out[j].branch_id == 0xFFFFFFFF && 
                cube_out[j].teller_id == raw_data[i].teller_id) {
                cube_out[j].total_amount += raw_data[i].total_amount;
                found = true;
                break;
            }
        }
        if (!found) {
            cube_out[count].branch_id = 0xFFFFFFFF;
            cube_out[count].teller_id = raw_data[i].teller_id;
            cube_out[count].total_amount = raw_data[i].total_amount;
            count++;
        }
    }
    
    // 4. Calculate grand total (ALL branches, ALL tellers: branch_id = teller_id = 0xFFFFFFFF)
    int32_t grand_total = 0;
    for (int i = 0; i < data_count; i++) {
        grand_total += raw_data[i].total_amount;
    }
    cube_out[count].branch_id = 0xFFFFFFFF;
    cube_out[count].teller_id = 0xFFFFFFFF;
    cube_out[count].total_amount = grand_total;
    count++;
    
    *cube_count_out = count;
    return 0;
}

// 8. DAG Hierarchical Lock Conversion Validator
int tsfi_gray_dag_lock_verify(tsfi_reuter_lock_mode parent_mode, tsfi_reuter_lock_mode child_mode) {
    // Jim Gray's DAG Rule:
    // To lock a child in S or IS mode, the parent must be locked in IS or IX.
    // To lock a child in X, IX, or SIX mode, the parent must be locked in IX or SIX.
    if (child_mode == LOCK_MODE_S || child_mode == LOCK_MODE_IS) {
        if (parent_mode == LOCK_MODE_IS || parent_mode == LOCK_MODE_IX) {
            return 0; // Authorized!
        }
    }
    
    if (child_mode == LOCK_MODE_X || child_mode == LOCK_MODE_IX || child_mode == LOCK_MODE_SIX) {
        if (parent_mode == LOCK_MODE_IX || parent_mode == LOCK_MODE_SIX || parent_mode == LOCK_MODE_X) {
            return 0; // Authorized!
        }
    }
    
    return -1; // Unauthorized lock conversion / hierarchy violation
}

// 9. Heartbeat-Based Backup Monitor (Process Pair Active Liveness)
int tsfi_gray_pp_heartbeat_check(tsfi_gray_heartbeat_monitor *mon, uint64_t current_time, bool *trigger_failover) {
    if (!mon || !trigger_failover) return -1;
    
    uint64_t idle = current_time > mon->last_heartbeat_time ? 
                    current_time - mon->last_heartbeat_time : 0;
    
    if (idle >= mon->heartbeat_timeout) {
        *trigger_failover = true;
    } else {
        *trigger_failover = false;
    }
    
    return 0;
}
