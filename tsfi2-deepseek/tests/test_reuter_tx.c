#include "tsfi_reuter_tx.h"
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
    printf("[INFO] Starting Andreas Reuter Transaction Compliance Test (Standard+MVCC)...\n");
    
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
    assert(dirty_count == 1);
    
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
    rc = tsfi_reuter_savepoint_create(&tx_ctx, "SP_BETA", 77);
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
    
    uint64_t g_lsn1 = 0, g_lsn2 = 0;
    rc = tsfi_reuter_group_commit_queue(&gc, 1006, 0, 3, before, after, &g_lsn1);
    assert(rc == 0);
    rc = tsfi_reuter_group_commit_queue(&gc, 1007, 4, 3, before, after, &g_lsn2);
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
    assert(rc == 0); // Escalated!
    assert(locks[0].requests[0].mode == LOCK_MODE_X); // First escalated to X
    assert(locks[1].request_count == 0); // Released others
    
    // Test 11: Wait-For Graph (WFG) Deadlock Detection
    tsfi_reuter_wfg_edge wfg[2];
    wfg[0].waiting_tx_id = 201; wfg[0].holding_tx_id = 202; // 201 waits for 202
    wfg[1].waiting_tx_id = 202; wfg[1].holding_tx_id = 201; // 202 waits for 201 (deadlock!)
    
    uint32_t victim = 0;
    rc = tsfi_reuter_wfg_detect_deadlock(wfg, 2, &victim);
    assert(rc == 0); // Deadlock cycle found!
    assert(victim == 201); // 201 selected as lower TX ID victim
    
    // Test 12: MVCC snapshot isolation & version sweep
    tsfi_reuter_version *mvcc_head = NULL;
    uint8_t data_v1[4] = {1, 1, 1, 1};
    uint8_t data_v2[4] = {2, 2, 2, 2};
    
    rc = tsfi_reuter_mvcc_write(&mvcc_head, 10, data_v1, 4);
    assert(rc == 0);
    rc = tsfi_reuter_mvcc_write(&mvcc_head, 20, data_v2, 4);
    assert(rc == 0);
    
    uint8_t read_buf[4];
    // Reader at Start LSN = 15 should read version v1 (LSN 10)
    rc = tsfi_reuter_mvcc_read(mvcc_head, 15, read_buf, 4);
    assert(rc == 0);
    assert(read_buf[0] == 1);
    
    // Reader at Start LSN = 25 should read version v2 (LSN 20)
    rc = tsfi_reuter_mvcc_read(mvcc_head, 25, read_buf, 4);
    assert(rc == 0);
    assert(read_buf[0] == 2);
    
    // Sweep older versions where min active LSN is 20 (v1 at LSN 10 is no longer needed)
    rc = tsfi_reuter_mvcc_sweep(&mvcc_head, 20);
    assert(rc == 0);
    
    // Check that LSN 10 is swept, but LSN 20 is still readable at LSN 20
    rc = tsfi_reuter_mvcc_read(mvcc_head, 20, read_buf, 4);
    assert(rc == 0);
    assert(read_buf[0] == 2);
    
    // Free remaining list items
    while (mvcc_head) {
        tsfi_reuter_version *tmp = mvcc_head;
        mvcc_head = mvcc_head->next;
        free(tmp);
    }
    
    close(fd);
    unlink(temp_log);
    
    printf("[SUCCESS] Andreas Reuter Transaction Compliance Test (Standard+MVCC) Passed!\n");
    return 0;
}
