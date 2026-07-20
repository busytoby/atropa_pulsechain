#include "tsfi_reuter_tx.h"
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void) {
    printf("[INFO] Starting Andreas Reuter Transaction Compliance Test (Extended)...\n");
    
    tsfi_reuter_init();
    assert(tsfi_reuter_get_global_lsn() == 0);
    
    // Test 1: Write-Ahead Logging (WAL)
    const char *temp_log = "tmp/reuter_tx.log";
    int fd = open(temp_log, O_RDWR | O_CREAT | O_TRUNC, 0644);
    assert(fd >= 0);
    
    uint8_t before[10] = {1, 2, 3};
    uint8_t after[10] = {1, 2, 9};
    uint64_t lsn1 = 0;
    
    int rc = tsfi_reuter_write_wal(fd, 1001, 12, 3, before, after, &lsn1);
    assert(rc == 0);
    assert(lsn1 == 1);
    assert(tsfi_reuter_get_global_lsn() == 1);
    
    // Test 2: Compensation Log Record (CLR)
    uint64_t lsn2 = 0;
    rc = tsfi_reuter_write_clr(fd, 1001, lsn1, 12, 3, before, &lsn2);
    assert(rc == 0);
    assert(lsn2 == 2);
    assert(tsfi_reuter_get_global_lsn() == 2);
    
    // Test 3: Physiological Page Logging
    tsfi_reuter_page page;
    memset(&page, 0, sizeof(tsfi_reuter_page));
    page.page_id = 0;
    
    uint8_t new_data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    rc = tsfi_reuter_page_write(&page, fd, 1001, 0, 4, new_data);
    assert(rc == 0);
    assert(page.page_lsn == 3);
    assert(memcmp(page.data, new_data, 4) == 0);
    
    // Test 4: ARIES Crash Recovery (Analysis, Redo, Undo)
    tsfi_reuter_tx_entry tx_table[16];
    tsfi_reuter_dirty_page dirty_table[16];
    int tx_count = 0;
    int dirty_count = 0;
    
    // Simulate crash rollback of active transactions
    tsfi_reuter_page dirty_page;
    memset(&dirty_page, 0, sizeof(tsfi_reuter_page));
    dirty_page.page_id = 0;
    dirty_page.page_lsn = 0; // Simulate old dirty page on disk
    
    rc = tsfi_reuter_aries_recover(fd, &dirty_page, 1, tx_table, &tx_count, dirty_table, &dirty_count);
    assert(rc == 0);
    assert(tx_count == 1);
    assert(dirty_count == 1);
    assert(dirty_page.page_lsn == 3); // Redo applied the write LSN 3
    assert(memcmp(dirty_page.data, new_data, 4) == 0);
    
    close(fd);
    unlink(temp_log);
    
    // Test 5: Hierarchical Intent Locking Compatibility
    tsfi_reuter_lock_head db_lock;
    memset(&db_lock, 0, sizeof(tsfi_reuter_lock_head));
    db_lock.resource_id = 99;
    
    // Acquire Intent Shared (IS) for TX1
    rc = tsfi_reuter_lock_acquire(&db_lock, 101, LOCK_MODE_IS);
    assert(rc == 0);
    assert(db_lock.requests[0].granted == true);
    
    // Acquire Intent Exclusive (IX) for TX2 (compatible with IS)
    rc = tsfi_reuter_lock_acquire(&db_lock, 102, LOCK_MODE_IX);
    assert(rc == 0);
    assert(db_lock.requests[1].granted == true);
    
    // Acquire Shared (S) for TX3 (compatible with IS, incompatible with IX)
    rc = tsfi_reuter_lock_acquire(&db_lock, 103, LOCK_MODE_S);
    assert(rc == 1); // Queued (not granted)
    assert(db_lock.requests[2].granted == false);
    
    // Release TX2 (IX) - should automatically grant TX3 (S)
    rc = tsfi_reuter_lock_release(&db_lock, 102);
    assert(rc == 0);
    assert(db_lock.requests[1].granted == true); // Request 1 (was TX3) is now granted!
    
    // Test 6: LU6.2 Two-Phase Commit (2PC)
    tsfi_reuter_2pc_coordinator coord;
    tsfi_reuter_2pc_init(&coord, 1002);
    tsfi_reuter_2pc_add_participant(&coord, 101);
    tsfi_reuter_2pc_add_participant(&coord, 102);
    
    tsfi_reuter_2pc_vote(&coord, 101, true);
    tsfi_reuter_2pc_vote(&coord, 102, true);
    
    bool is_committed = false;
    rc = tsfi_reuter_2pc_finalize(&coord, &is_committed);
    assert(rc == 0);
    assert(is_committed == true);
    
    printf("[SUCCESS] Andreas Reuter Transaction Compliance Test (Extended) Passed!\n");
    return 0;
}
