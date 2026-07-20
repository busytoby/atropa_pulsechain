#include "tsfi_reuter_tx.h"
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
    printf("[INFO] Starting Andreas Reuter Transaction Compliance Test (Comprehensive)...\n");
    
    tsfi_reuter_init();
    assert(tsfi_reuter_get_global_lsn() == 0);
    
    // Test 1: Write-Ahead Logging (WAL)
    const char *temp_log = "tmp/reuter_tx_comprehensive.log";
    int fd = open(temp_log, O_RDWR | O_CREAT | O_TRUNC, 0644);
    assert(fd >= 0);
    
    uint8_t before[10] = {1, 2, 3};
    uint8_t after[10] = {1, 2, 9};
    uint64_t lsn1 = 0;
    
    int rc = tsfi_reuter_write_wal(fd, 1001, 12, 3, before, after, &lsn1);
    assert(rc == 0);
    assert(lsn1 == 1);
    
    // Test 2: Compensation Log Record (CLR)
    uint64_t lsn2 = 0;
    rc = tsfi_reuter_write_clr(fd, 1001, lsn1, 12, 3, before, &lsn2);
    assert(rc == 0);
    assert(lsn2 == 2);
    
    // Test 3: Physiological Page Logging
    tsfi_reuter_page page;
    memset(&page, 0, sizeof(tsfi_reuter_page));
    page.page_id = 0;
    
    uint8_t new_data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    rc = tsfi_reuter_page_write(&page, fd, 1001, 0, 4, new_data);
    assert(rc == 0);
    assert(page.page_lsn == 3);
    
    // Test 4: ARIES Crash Recovery (Analysis, Redo, Undo)
    tsfi_reuter_tx_entry tx_table[16];
    tsfi_reuter_dirty_page dirty_table[16];
    int tx_count = 0;
    int dirty_count = 0;
    
    tsfi_reuter_page dirty_page;
    memset(&dirty_page, 0, sizeof(tsfi_reuter_page));
    dirty_page.page_id = 0;
    dirty_page.page_lsn = 0; 
    
    rc = tsfi_reuter_aries_recover(fd, &dirty_page, 1, tx_table, &tx_count, dirty_table, &dirty_count);
    assert(rc == 0);
    assert(tx_count == 1);
    
    // Test 5: Hierarchical Intent Locking Compatibility
    tsfi_reuter_lock_head db_lock;
    memset(&db_lock, 0, sizeof(tsfi_reuter_lock_head));
    db_lock.resource_id = 99;
    
    rc = tsfi_reuter_lock_acquire(&db_lock, 101, LOCK_MODE_IS);
    assert(rc == 0);
    rc = tsfi_reuter_lock_acquire(&db_lock, 102, LOCK_MODE_IX);
    assert(rc == 0);
    rc = tsfi_reuter_lock_acquire(&db_lock, 103, LOCK_MODE_S);
    assert(rc == 1); // Queued
    
    rc = tsfi_reuter_lock_release(&db_lock, 102);
    assert(rc == 0);
    
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
    
    // Test 7: Savepoints and Nested Rollback
    tsfi_reuter_tx_context tx_ctx;
    memset(&tx_ctx, 0, sizeof(tsfi_reuter_tx_context));
    tx_ctx.transaction_id = 1005;
    
    rc = tsfi_reuter_savepoint_create(&tx_ctx, "SP_ALPHA", 55);
    assert(rc == 0);
    
    uint64_t rolled_lsn = tsfi_reuter_savepoint_rollback(&tx_ctx, "SP_ALPHA");
    assert(rolled_lsn == 55);
    
    // Test 8: Fuzzy Checkpoint
    uint64_t chk_lsn = 0;
    rc = tsfi_reuter_write_checkpoint(fd, tx_table, tx_count, dirty_table, dirty_count, &chk_lsn);
    assert(rc == 0);
    
    // Test 9: Group Commit
    tsfi_reuter_group_commit gc;
    tsfi_reuter_group_commit_init(&gc, fd);
    
    uint64_t g_lsn1 = 0;
    rc = tsfi_reuter_group_commit_queue(&gc, 1006, 0, 3, before, after, &g_lsn1);
    assert(rc == 0);
    rc = tsfi_reuter_group_commit_flush(&gc);
    assert(rc == 0);
    
    // Test 10: Lock Escalation
    tsfi_reuter_lock_head locks[4];
    memset(locks, 0, sizeof(locks));
    for (int i = 0; i < 4; i++) {
        locks[i].resource_id = i;
        tsfi_reuter_lock_acquire(&locks[i], 1008, LOCK_MODE_S);
    }
    rc = tsfi_reuter_lock_escalate(locks, 4, 1008);
    assert(rc == 0);
    
    // Test 11: Lock Conversion & Upgrade
    tsfi_reuter_lock_head res_lock;
    memset(&res_lock, 0, sizeof(tsfi_reuter_lock_head));
    res_lock.resource_id = 1010;
    
    rc = tsfi_reuter_lock_acquire(&res_lock, 1011, LOCK_MODE_S);
    assert(rc == 0);
    // Upgrade S lock to X lock for same transaction
    rc = tsfi_reuter_lock_upgrade(&res_lock, 1011, LOCK_MODE_X);
    assert(rc == 0);
    assert(res_lock.requests[0].mode == LOCK_MODE_X);
    
    // Test 12: SS2PL release all locks
    tsfi_reuter_lock_head ss2pl_locks[2];
    memset(ss2pl_locks, 0, sizeof(ss2pl_locks));
    tsfi_reuter_lock_acquire(&ss2pl_locks[0], 1012, LOCK_MODE_S);
    tsfi_reuter_lock_acquire(&ss2pl_locks[1], 1012, LOCK_MODE_X);
    
    rc = tsfi_reuter_ss2pl_release_all(ss2pl_locks, 2, 1012);
    assert(rc == 0);
    assert(ss2pl_locks[0].request_count == 0);
    assert(ss2pl_locks[1].request_count == 0);
    
    // Test 13: Log Reclamation / Truncation Sweeper
    // Log contains multiple records. Truncate records before LSN = 3.
    // LSNs created in this test run:
    // WAL 1: LSN 1
    // CLR 2: LSN 2
    // Page write 3: LSN 3
    // Checkpoint 4: LSN 4
    // Group commit 5: LSN 5
    // Let's sweep log files before LSN 3 (only LSN 3, 4, 5 should remain)
    rc = tsfi_reuter_log_truncate(fd, 3, temp_log);
    assert(rc == 0);
    
    // Verify file content after truncation
    close(fd);
    fd = open(temp_log, O_RDONLY);
    assert(fd >= 0);
    
    tsfi_reuter_log_record record;
    int read_count = 0;
    while (read(fd, &record, sizeof(tsfi_reuter_log_record)) == sizeof(tsfi_reuter_log_record)) {
        assert(record.lsn >= 3);
        read_count++;
    }
    assert(read_count == 4);
    
    close(fd);
    unlink(temp_log);
    
    printf("[SUCCESS] Andreas Reuter Transaction Compliance Test (Comprehensive) Passed!\n");
    return 0;
}
