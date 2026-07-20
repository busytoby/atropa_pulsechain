#include "tsfi_gray_tx.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("[INFO] Starting Jim Gray Transaction Compliance Test (Extended)...\n");
    
    // Test 1: Degrees of Consistency locking
    tsfi_reuter_lock_head lock_head;
    memset(&lock_head, 0, sizeof(tsfi_reuter_lock_head));
    lock_head.resource_id = 100;
    
    // Degree 0 read does not acquire locks
    int rc = tsfi_gray_lock_read(&lock_head, 9001, CONSISTENCY_DEGREE_0);
    assert(rc == 0);
    assert(lock_head.request_count == 0);
    
    // Degree 1 read acquires short lock
    rc = tsfi_gray_lock_read(&lock_head, 9002, CONSISTENCY_DEGREE_1);
    assert(rc == 0);
    assert(lock_head.request_count == 1);
    assert(lock_head.requests[0].granted == true);
    
    rc = tsfi_gray_lock_read_release(&lock_head, 9002, CONSISTENCY_DEGREE_1);
    assert(rc == 0);
    assert(lock_head.request_count == 0); // Short lock released immediately!
    
    // Degree 2/3 read acquires long lock (held until end of transaction)
    rc = tsfi_gray_lock_read(&lock_head, 9003, CONSISTENCY_DEGREE_3);
    assert(rc == 0);
    assert(lock_head.request_count == 1);
    
    rc = tsfi_gray_lock_read_release(&lock_head, 9003, CONSISTENCY_DEGREE_3);
    assert(rc == 0);
    assert(lock_head.request_count == 1); // Held! Not released!
    
    tsfi_reuter_lock_release(&lock_head, 9003); // Explicit clean up
    
    // Test 2: Five-Minute Rule Cache Manager
    tsfi_gray_cache_manager cm;
    tsfi_gray_cache_init(&cm);
    
    tsfi_gray_cache_entry *entry = NULL;
    // Access page 10 at time 100
    rc = tsfi_gray_cache_access(&cm, 10, 100, &entry);
    assert(rc == 1); // Miss and load
    assert(entry != NULL && entry->page_id == 10);
    
    // Access page 10 again at time 150 (hit)
    rc = tsfi_gray_cache_access(&cm, 10, 150, &entry);
    assert(rc == 0); // Hit
    assert(entry->last_access == 150);
    
    // Sweep cache at time 400 (idle = 250s, less than 300s limit -> no eviction)
    uint32_t evicted_id = 0;
    rc = tsfi_gray_cache_evict_sweep(&cm, 400, &evicted_id);
    assert(rc == 1); // Sweep returned no evictions
    
    // Sweep cache at time 460 (idle = 310s, greater than 300s limit -> evict page 10)
    rc = tsfi_gray_cache_evict_sweep(&cm, 460, &evicted_id);
    assert(rc == 0); // Evicted!
    assert(evicted_id == 10);
    
    // Test 3: Linear Hashing Temp Table Splits
    tsfi_gray_linear_hash lh;
    rc = tsfi_gray_lh_init(&lh, 2);
    assert(rc == 0);
    
    rc = tsfi_gray_lh_insert(&lh, 1, 100);
    assert(rc == 0);
    rc = tsfi_gray_lh_insert(&lh, 3, 300);
    assert(rc == 0);
    rc = tsfi_gray_lh_insert(&lh, 5, 500); // Trigger split of bucket 0
    assert(rc == 0);
    
    uint32_t lookup_val = 0;
    rc = tsfi_gray_lh_lookup(&lh, 3, &lookup_val);
    assert(rc == 0 && lookup_val == 300);
    
    tsfi_gray_lh_destroy(&lh);
    
    // Test 4: Fault-Tolerant Process Pairs Failover
    tsfi_gray_process_pair pair;
    memset(&pair, 0, sizeof(tsfi_gray_process_pair));
    
    tsfi_reuter_page pri_page;
    memset(&pri_page, 0, sizeof(tsfi_reuter_page));
    pri_page.page_id = 8;
    strcpy((char *)pri_page.data, "Tandem Primary Memory Content");
    
    // Sync Primary state to Backup
    rc = tsfi_gray_pp_checkpoint_sync(&pair, 101, 45, &pri_page);
    assert(rc == 0);
    assert(pair.active_tx_id == 101);
    
    // Trigger primary node crash and failover takeover
    tsfi_reuter_page backup_active_page;
    rc = tsfi_gray_pp_failover(&pair, &backup_active_page);
    assert(rc == 0);
    assert(pair.primary_failed == true);
    assert(strcmp((char *)backup_active_page.data, "Tandem Primary Memory Content") == 0);
    
    // Test 5: Debit-Credit (TPC-A workload simulation)
    tsfi_gray_tpca_state bank_state;
    bank_state.branch_balance = 5000;
    bank_state.teller_balance = 500;
    bank_state.account_balance = 10000;
    
    int history_record = 0;
    rc = tsfi_gray_tpca_exec(&bank_state, -100, &history_record);
    assert(rc == 0);
    assert(bank_state.account_balance == 9900);
    assert(bank_state.teller_balance == 400);
    assert(bank_state.branch_balance == 4900);
    assert(history_record == 15200); // 9900 + 400 + 4900
    
    // Test 6: Cascading Abort Chain Tracker
    tsfi_gray_abort_tracker tracker;
    tsfi_gray_abort_tracker_init(&tracker);
    
    // tx 1001 writes data, tx 1002 reads it (dirty dependency)
    rc = tsfi_gray_abort_add_dep(&tracker, 1001, 1002);
    assert(rc == 0);
    
    // tx 1002 writes data, tx 1003 reads it (dirty dependency)
    rc = tsfi_gray_abort_add_dep(&tracker, 1002, 1003);
    assert(rc == 0);
    
    // tx 1001 aborts -> check if 1002 and 1003 cascade abort
    uint32_t cascaded_aborts[16];
    memset(cascaded_aborts, 0, sizeof(cascaded_aborts));
    int cascade_count = 0;
    
    rc = tsfi_gray_abort_cascade(&tracker, 1001, cascaded_aborts, &cascade_count);
    assert(rc == 0);
    assert(cascade_count == 2);
    assert(cascaded_aborts[0] == 1002);
    assert(cascaded_aborts[1] == 1003);
    
    printf("[SUCCESS] Jim Gray Transaction Compliance Test (Extended) Passed!\n");
    return 0;
}
