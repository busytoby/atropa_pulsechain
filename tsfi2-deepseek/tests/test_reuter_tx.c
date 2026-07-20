#include "tsfi_reuter_tx.h"
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
    printf("[INFO] Starting Andreas Reuter Transaction Compliance Test (Ultra-Comprehensive)...\n");
    
    tsfi_reuter_init();
    assert(tsfi_reuter_get_global_lsn() == 0);
    
    // Test 1: Write-Ahead Logging (WAL)
    const char *temp_log = "tmp/reuter_tx_ultra.log";
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
    
    // Test 13: Log Reclamation / Truncation Sweeper
    rc = tsfi_reuter_log_truncate(fd, 3, temp_log);
    assert(rc == 0);
    
    // Verify file content after truncation
    close(fd);
    fd = open(temp_log, O_RDWR);
    assert(fd >= 0);
    
    tsfi_reuter_log_record record;
    int read_count = 0;
    while (read(fd, &record, sizeof(tsfi_reuter_log_record)) == sizeof(tsfi_reuter_log_record)) {
        assert(record.lsn >= 3);
        read_count++;
    }
    assert(read_count == 4);
    
    // Test 14: Checkpoint-Driven Recovery Boot
    tsfi_reuter_page boot_page;
    memset(&boot_page, 0, sizeof(tsfi_reuter_page));
    boot_page.page_id = 0;
    boot_page.page_lsn = 0;
    
    rc = tsfi_reuter_aries_recover_from_checkpoint(fd, &boot_page, 1, tx_table, &tx_count, dirty_table, &dirty_count);
    assert(rc == 0);
    
    // Test 15: LU6.2 conversational Syncpoint Handshake loop
    tsfi_reuter_2pc_coordinator sync_coord;
    tsfi_reuter_2pc_init(&sync_coord, 2001);
    tsfi_reuter_2pc_add_participant(&sync_coord, 101);
    
    char send_buf[32] = "REQ_COMMIT";
    char recv_buf[32] = "";
    int status = tsfi_reuter_lu62_syncpoint_handshake(&sync_coord, 101, send_buf, recv_buf);
    assert(status == LU62_STATE_SYNC_PENDING);
    assert(strcmp(recv_buf, "PREPARE") == 0);
    
    // Test 16: Selective subtransaction rollback
    tsfi_reuter_page sel_page;
    memset(&sel_page, 0, sizeof(tsfi_reuter_page));
    sel_page.page_id = 1;
    
    uint8_t data_step1[4] = {7, 7, 7, 7};
    uint8_t data_step2[4] = {9, 9, 9, 9};
    
    uint64_t target_boundary_lsn = 0;
    rc = tsfi_reuter_page_write(&sel_page, fd, 3001, 12, 4, data_step1);
    assert(rc == 0);
    target_boundary_lsn = sel_page.page_lsn;
    
    rc = tsfi_reuter_page_write(&sel_page, fd, 3001, 12, 4, data_step2);
    assert(rc == 0);
    
    rc = tsfi_reuter_selective_undo(fd, &sel_page, 1, 3001, target_boundary_lsn);
    assert(rc == 0);
    assert(memcmp(sel_page.data + 12, data_step1, 4) == 0);
    
    // Test 17: Presumed Commit vs. Presumed Abort
    tsfi_reuter_2pc_coordinator presumed_coord;
    tsfi_reuter_2pc_init(&presumed_coord, 3005);
    tsfi_reuter_2pc_add_participant(&presumed_coord, 101);
    tsfi_reuter_2pc_vote(&presumed_coord, 101, true); // Prepare Node
    
    bool p_commit = false;
    rc = tsfi_reuter_2pc_finalize_presumed(&presumed_coord, true, &p_commit); // Presumed Abort finalization
    assert(rc == 0 && p_commit == true);
    
    // Test 18: Doublewrite Buffer Torn Page Recovery
    const char *dw_log = "tmp/reuter_dw.log";
    int dw_fd = open(dw_log, O_RDWR | O_CREAT | O_TRUNC, 0644);
    assert(dw_fd >= 0);
    
    tsfi_reuter_doublewrite_buffer dwb;
    tsfi_reuter_doublewrite_init(&dwb);
    
    tsfi_reuter_page write_page;
    memset(&write_page, 0, sizeof(tsfi_reuter_page));
    write_page.page_id = 45;
    strcpy((char *)write_page.data, "Doublewrite Intact Block Data");
    
    rc = tsfi_reuter_doublewrite_flush(&dwb, dw_fd, &write_page);
    assert(rc == 0);
    
    // Corrupt page locally (torn page simulation)
    tsfi_reuter_page torn_page;
    memset(&torn_page, 0, sizeof(tsfi_reuter_page));
    torn_page.page_id = 45;
    strcpy((char *)torn_page.data, "Corrupted/Torn Block data...");
    
    // Restore page from Doublewrite buffer file copy
    rc = tsfi_reuter_doublewrite_recover_page(dw_fd, &torn_page);
    assert(rc == 0);
    assert(strcmp((char *)torn_page.data, "Doublewrite Intact Block Data") == 0);
    
    // Test 19: Background Page Cleaner WAL Check
    tsfi_reuter_page dirty_clean_page;
    memset(&dirty_clean_page, 0, sizeof(tsfi_reuter_page));
    dirty_clean_page.page_id = 2;
    dirty_clean_page.page_lsn = 85; // Page modified at LSN 85
    
    const char *db_file = "tmp/reuter_db.dat";
    int db_fd = open(db_file, O_RDWR | O_CREAT | O_TRUNC, 0644);
    assert(db_fd >= 0);
    
    // Flush page using page cleaner while global flushed LSN is only 80 (WAL violation!)
    rc = tsfi_reuter_page_cleaner_flush_page(&dirty_clean_page, db_fd, 80);
    assert(rc == -2); // Blocked!
    
    // Flush page using page cleaner while global flushed LSN is 90 (WAL satisfied!)
    rc = tsfi_reuter_page_cleaner_flush_page(&dirty_clean_page, db_fd, 90);
    assert(rc == 0); // Succeeded!
    
    close(db_fd);
    unlink(db_file);
    close(dw_fd);
    unlink(dw_log);
    
    close(fd);
    unlink(temp_log);
    
    printf("[SUCCESS] Andreas Reuter Transaction Compliance Test (Ultra-Comprehensive) Passed!\n");
    return 0;
}
