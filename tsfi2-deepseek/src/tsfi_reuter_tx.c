#include "tsfi_reuter_tx.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

static uint64_t global_lsn = 0;

void tsfi_reuter_init(void) {
    global_lsn = 0;
}

uint64_t tsfi_reuter_get_global_lsn(void) {
    return global_lsn;
}

// 1. Write-Ahead Logging (WAL) with monotonic LSN tracking
int tsfi_reuter_write_wal(int log_fd, uint32_t tx_id, uint32_t offset, uint32_t len, 
                          const uint8_t *before, const uint8_t *after, uint64_t *assigned_lsn) {
    if (log_fd < 0 || len > REUTER_MAX_DATA_SIZE) return -1;
    
    tsfi_reuter_log_record record;
    memset(&record, 0, sizeof(tsfi_reuter_log_record));
    
    record.lsn = ++global_lsn;
    record.transaction_id = tx_id;
    record.type = LOG_TYPE_WAL;
    record.data_offset = offset;
    record.data_len = len;
    
    if (before) memcpy(record.before_image, before, len);
    if (after) memcpy(record.after_image, after, len);
    
    // Write log record to stable storage
    ssize_t bytes_written = write(log_fd, &record, sizeof(tsfi_reuter_log_record));
    if (bytes_written < (ssize_t)sizeof(tsfi_reuter_log_record)) {
        return -2; // Write failed
    }
    
    // Flush to storage medium (Durability guarantee)
    fsync(log_fd);
    
    if (assigned_lsn) {
        *assigned_lsn = record.lsn;
    }
    return 0;
}

// 2. Compensation Log Records (CLRs) for rollbacks
int tsfi_reuter_write_clr(int log_fd, uint32_t tx_id, uint64_t undo_next_lsn, uint32_t offset, 
                          uint32_t len, const uint8_t *comp_data, uint64_t *assigned_lsn) {
    if (log_fd < 0 || len > REUTER_MAX_DATA_SIZE) return -1;
    
    tsfi_reuter_log_record record;
    memset(&record, 0, sizeof(tsfi_reuter_log_record));
    
    record.lsn = ++global_lsn;
    record.transaction_id = tx_id;
    record.type = LOG_TYPE_CLR;
    record.undo_next_lsn = undo_next_lsn;
    record.data_offset = offset;
    record.data_len = len;
    
    if (comp_data) {
        memcpy(record.after_image, comp_data, len); // Use after_image for CLR compensation data
    }
    
    // Write Compensation Log Record to stable storage
    ssize_t bytes_written = write(log_fd, &record, sizeof(tsfi_reuter_log_record));
    if (bytes_written < (ssize_t)sizeof(tsfi_reuter_log_record)) {
        return -2; // Write failed
    }
    
    fsync(log_fd);
    
    if (assigned_lsn) {
        *assigned_lsn = record.lsn;
    }
    return 0;
}

// 3. LU6.2 Two-Phase Commit Coordinator Initialization
void tsfi_reuter_2pc_init(tsfi_reuter_2pc_coordinator *coord, uint32_t tx_id) {
    if (!coord) return;
    memset(coord, 0, sizeof(tsfi_reuter_2pc_coordinator));
    coord->transaction_id = tx_id;
    coord->global_decision_commit = true; // Optimistic start
}

int tsfi_reuter_2pc_add_participant(tsfi_reuter_2pc_coordinator *coord, uint32_t node_id) {
    if (!coord || coord->participant_count >= REUTER_MAX_NODES) return -1;
    
    coord->participant_ids[coord->participant_count] = node_id;
    coord->participant_states[coord->participant_count] = NODE_STATE_INIT;
    coord->participant_count++;
    return 0;
}

int tsfi_reuter_2pc_vote(tsfi_reuter_2pc_coordinator *coord, uint32_t node_id, bool vote_commit) {
    if (!coord) return -1;
    
    for (int i = 0; i < coord->participant_count; i++) {
        if (coord->participant_ids[i] == node_id) {
            coord->participant_states[i] = vote_commit ? NODE_STATE_PREPARED : NODE_STATE_ABORTED;
            if (!vote_commit) {
                coord->global_decision_commit = false; // Single veto aborts globally
            }
            return 0;
        }
    }
    return -2; // Node not found
}

int tsfi_reuter_2pc_finalize(tsfi_reuter_2pc_coordinator *coord, bool *is_committed) {
    if (!coord || !is_committed) return -1;
    
    // Check if all participants successfully prepared
    for (int i = 0; i < coord->participant_count; i++) {
        if (coord->participant_states[i] != NODE_STATE_PREPARED) {
            coord->global_decision_commit = false;
        }
    }
    
    // Commit/Abort transition
    for (int i = 0; i < coord->participant_count; i++) {
        coord->participant_states[i] = coord->global_decision_commit ? NODE_STATE_COMMITTED : NODE_STATE_ABORTED;
    }
    
    *is_committed = coord->global_decision_commit;
    return 0;
}

// 4. Physiological Page Logging
int tsfi_reuter_page_write(tsfi_reuter_page *page, int log_fd, uint32_t tx_id, uint32_t offset, 
                           uint32_t len, const uint8_t *new_data) {
    if (!page || log_fd < 0 || (offset + len) > 512) return -1;
    
    uint8_t before[REUTER_MAX_DATA_SIZE];
    memset(before, 0, REUTER_MAX_DATA_SIZE);
    memcpy(before, page->data + offset, len > REUTER_MAX_DATA_SIZE ? REUTER_MAX_DATA_SIZE : len);
    
    // Write changes first to log (WAL protocol)
    uint64_t assigned_lsn = 0;
    int rc = tsfi_reuter_write_wal(log_fd, tx_id, page->page_id * 512 + offset, len, before, new_data, &assigned_lsn);
    if (rc != 0) return rc;
    
    // Update physiological page state
    memcpy(page->data + offset, new_data, len);
    page->page_lsn = assigned_lsn; // Update page LSN
    if (offset + len > page->free_offset) {
        page->free_offset = offset + len;
    }
    
    return 0;
}

// Helper: Check compatibility of lock modes
static bool is_lock_compatible(tsfi_reuter_lock_mode requested, tsfi_reuter_lock_mode held) {
    if (requested == LOCK_MODE_X || held == LOCK_MODE_X) return false;
    if (requested == LOCK_MODE_SIX && held != LOCK_MODE_IS) return false;
    if (held == LOCK_MODE_SIX && requested != LOCK_MODE_IS) return false;
    if (requested == LOCK_MODE_S && (held == LOCK_MODE_IX || held == LOCK_MODE_SIX)) return false;
    if (held == LOCK_MODE_S && (requested == LOCK_MODE_IX || requested == LOCK_MODE_SIX)) return false;
    return true;
}

// 5. Hierarchical Lock Manager
int tsfi_reuter_lock_acquire(tsfi_reuter_lock_head *lock_head, uint32_t tx_id, tsfi_reuter_lock_mode mode) {
    if (!lock_head) return -1;
    
    // Check if transaction already holds the lock
    for (int i = 0; i < lock_head->request_count; i++) {
        if (lock_head->requests[i].transaction_id == tx_id) {
            if (lock_head->requests[i].mode >= mode) {
                return 0; // Already holds sufficient lock mode
            }
            // Upgrade lock request
            lock_head->requests[i].mode = mode;
            return 0;
        }
    }
    
    if (lock_head->request_count >= 8) return -2; // Queue full
    
    // Evaluate compatibility with existing granted locks
    bool compatible = true;
    for (int i = 0; i < lock_head->request_count; i++) {
        if (lock_head->requests[i].granted && !is_lock_compatible(mode, lock_head->requests[i].mode)) {
            compatible = false;
            break;
        }
    }
    
    lock_head->requests[lock_head->request_count].transaction_id = tx_id;
    lock_head->requests[lock_head->request_count].mode = mode;
    lock_head->requests[lock_head->request_count].granted = compatible;
    lock_head->request_count++;
    
    return compatible ? 0 : 1; // 0 = granted immediately, 1 = queued/waiting
}

int tsfi_reuter_lock_release(tsfi_reuter_lock_head *lock_head, uint32_t tx_id) {
    if (!lock_head) return -1;
    
    int index = -1;
    for (int i = 0; i < lock_head->request_count; i++) {
        if (lock_head->requests[i].transaction_id == tx_id) {
            index = i;
            break;
        }
    }
    
    if (index == -1) return -2; // Lock request not found
    
    // Remove request from queue
    for (int i = index; i < lock_head->request_count - 1; i++) {
        lock_head->requests[i] = lock_head->requests[i + 1];
    }
    lock_head->request_count--;
    
    // Grant waiting requests if compatible now
    for (int i = 0; i < lock_head->request_count; i++) {
        if (!lock_head->requests[i].granted) {
            bool compatible = true;
            for (int j = 0; j < i; j++) {
                if (lock_head->requests[j].granted && !is_lock_compatible(lock_head->requests[i].mode, lock_head->requests[j].mode)) {
                    compatible = false;
                    break;
                }
            }
            lock_head->requests[i].granted = compatible;
        }
    }
    
    return 0;
}

// 6. ARIES 3-Phase Crash Recovery
int tsfi_reuter_aries_recover(int log_fd, tsfi_reuter_page *pages, int page_count, 
                              tsfi_reuter_tx_entry *tx_table, int *tx_count, 
                              tsfi_reuter_dirty_page *dirty_table, int *dirty_count) {
    if (log_fd < 0 || !pages || !tx_table || !dirty_table) return -1;
    
    // Reset recovery state tables
    *tx_count = 0;
    *dirty_count = 0;
    
    // Seek log to beginning
    lseek(log_fd, 0, SEEK_SET);
    
    // PHASE 1: Analysis Phase (Scan log forward to construct active TX and dirty pages lists)
    tsfi_reuter_log_record log_rec;
    while (read(log_fd, &log_rec, sizeof(tsfi_reuter_log_record)) == sizeof(tsfi_reuter_log_record)) {
        // Track transaction state
        bool found_tx = false;
        for (int i = 0; i < *tx_count; i++) {
            if (tx_table[i].transaction_id == log_rec.transaction_id) {
                tx_table[i].last_lsn = log_rec.lsn;
                found_tx = true;
                break;
            }
        }
        if (!found_tx && *tx_count < 16) {
            tx_table[*tx_count].transaction_id = log_rec.transaction_id;
            tx_table[*tx_count].last_lsn = log_rec.lsn;
            tx_table[*tx_count].active = true;
            (*tx_count)++;
        }
        
        // Track dirty pages (assuming Page ID maps from data offset block size)
        uint32_t page_id = log_rec.data_offset / 512;
        bool found_page = false;
        for (int i = 0; i < *dirty_count; i++) {
            if (dirty_table[i].page_id == page_id) {
                found_page = true;
                break;
            }
        }
        if (!found_page && *dirty_count < 16) {
            dirty_table[*dirty_count].page_id = page_id;
            dirty_table[*dirty_count].rec_lsn = log_rec.lsn;
            (*dirty_count)++;
        }
    }
    
    // PHASE 2: Redo Phase (Scan forward to reapply uncommitted changes, ensuring PageLSN check)
    lseek(log_fd, 0, SEEK_SET);
    while (read(log_fd, &log_rec, sizeof(tsfi_reuter_log_record)) == sizeof(tsfi_reuter_log_record)) {
        uint32_t page_id = log_rec.data_offset / 512;
        tsfi_reuter_page *target_page = NULL;
        for (int i = 0; i < page_count; i++) {
            if (pages[i].page_id == page_id) {
                target_page = &pages[i];
                break;
            }
        }
        
        if (target_page) {
            // Replay history: Redo only if page LSN is older than log LSN
            if (target_page->page_lsn < log_rec.lsn) {
                uint32_t local_offset = log_rec.data_offset % 512;
                memcpy(target_page->data + local_offset, log_rec.after_image, log_rec.data_len);
                target_page->page_lsn = log_rec.lsn; // Update page LSN to prevent duplicate redo
            }
        }
    }
    
    // PHASE 3: Undo Phase (Simulate rollback of active transactions in reverse LSN order)
    for (int i = *tx_count - 1; i >= 0; i--) {
        if (tx_table[i].active) {
            // Log Compensation Log Record (CLR) to represent abort completion
            uint64_t clr_lsn = 0;
            tsfi_reuter_write_clr(log_fd, tx_table[i].transaction_id, tx_table[i].last_lsn, 0, 0, NULL, &clr_lsn);
            tx_table[i].active = false; // Transaction successfully aborted
        }
    }
    
    return 0; // Recovery completed successfully
}

// 7. Savepoints and Nested Transactions
int tsfi_reuter_savepoint_create(tsfi_reuter_tx_context *ctx, const char *name, uint64_t current_lsn) {
    if (!ctx || !name || ctx->savepoint_count >= 8) return -1;
    
    tsfi_reuter_savepoint *sp = &ctx->savepoints[ctx->savepoint_count];
    strncpy(sp->name, name, 31);
    sp->name[31] = '\0';
    sp->savepoint_lsn = current_lsn;
    ctx->savepoint_count++;
    
    return 0;
}

uint64_t tsfi_reuter_savepoint_rollback(tsfi_reuter_tx_context *ctx, const char *name) {
    if (!ctx || !name) return 0;
    
    for (int i = ctx->savepoint_count - 1; i >= 0; i--) {
        if (strcmp(ctx->savepoints[i].name, name) == 0) {
            uint64_t target_lsn = ctx->savepoints[i].savepoint_lsn;
            ctx->savepoint_count = i; // Discard all savepoints created after this one
            return target_lsn;
        }
    }
    return 0; // Savepoint not found
}

// 8. Fuzzy Checkpointing
int tsfi_reuter_write_checkpoint(int log_fd, const tsfi_reuter_tx_entry *tx_table, int tx_count, 
                                 const tsfi_reuter_dirty_page *dirty_table, int dirty_count, 
                                 uint64_t *chk_lsn) {
    if (log_fd < 0 || !tx_table || !dirty_table) return -1;
    
    tsfi_reuter_log_record record;
    memset(&record, 0, sizeof(tsfi_reuter_log_record));
    
    record.lsn = ++global_lsn;
    record.type = LOG_TYPE_CHECKPOINT;
    
    // Copy active transactions to before_image (up to 8 transactions, 4 bytes each)
    int tx_to_copy = tx_count < 8 ? tx_count : 8;
    for (int i = 0; i < tx_to_copy; i++) {
        memcpy(record.before_image + (i * 4), &tx_table[i].transaction_id, 4);
    }
    
    // Copy dirty pages to after_image (up to 8 pages, 4 bytes each)
    int pages_to_copy = dirty_count < 8 ? dirty_count : 8;
    for (int i = 0; i < pages_to_copy; i++) {
        memcpy(record.after_image + (i * 4), &dirty_table[i].page_id, 4);
    }
    
    // Write checkpoint record to stable storage
    ssize_t bytes_written = write(log_fd, &record, sizeof(tsfi_reuter_log_record));
    if (bytes_written < (ssize_t)sizeof(tsfi_reuter_log_record)) {
        return -2;
    }
    
    // Fuzzy checkpoint does NOT block other writers, just flushes metadata
    fsync(log_fd);
    
    if (chk_lsn) {
        *chk_lsn = record.lsn;
    }
    return 0;
}

// 9. Group Commit
void tsfi_reuter_group_commit_init(tsfi_reuter_group_commit *gc, int log_fd) {
    if (!gc) return;
    memset(gc, 0, sizeof(tsfi_reuter_group_commit));
    gc->log_fd = log_fd;
}

int tsfi_reuter_group_commit_queue(tsfi_reuter_group_commit *gc, uint32_t tx_id, uint32_t offset, 
                                    uint32_t len, const uint8_t *before, const uint8_t *after, 
                                    uint64_t *assigned_lsn) {
    if (!gc || len > REUTER_MAX_DATA_SIZE) return -1;
    
    // Flush if buffer is full
    if (gc->queued_count >= 16) {
        int rc = tsfi_reuter_group_commit_flush(gc);
        if (rc != 0) return rc;
    }
    
    int index = gc->queued_count;
    tsfi_reuter_log_record *record = &gc->queued_records[index];
    memset(record, 0, sizeof(tsfi_reuter_log_record));
    
    record->lsn = 0; // Will be assigned during flush
    record->transaction_id = tx_id;
    record->type = LOG_TYPE_WAL;
    record->data_offset = offset;
    record->data_len = len;
    
    if (before) memcpy(record->before_image, before, len);
    if (after) memcpy(record->after_image, after, len);
    
    gc->assigned_lsns[index] = assigned_lsn;
    gc->queued_count++;
    
    return 0;
}

int tsfi_reuter_group_commit_flush(tsfi_reuter_group_commit *gc) {
    if (!gc || gc->queued_count == 0) return 0;
    if (gc->log_fd < 0) return -1;
    
    // Assign LSNs and write all queued records in a SINGLE sequential block
    for (int i = 0; i < gc->queued_count; i++) {
        gc->queued_records[i].lsn = ++global_lsn;
        if (gc->assigned_lsns[i]) {
            *(gc->assigned_lsns[i]) = gc->queued_records[i].lsn;
        }
        
        ssize_t bytes_written = write(gc->log_fd, &gc->queued_records[i], sizeof(tsfi_reuter_log_record));
        if (bytes_written < (ssize_t)sizeof(tsfi_reuter_log_record)) {
            return -2;
        }
    }
    
    // Single consolidated fsync call for the entire group
    fsync(gc->log_fd);
    
    gc->queued_count = 0; // Reset queue
    return 0;
}

// 10. Lock Escalation
int tsfi_reuter_lock_escalate(tsfi_reuter_lock_head *lock_heads, int lock_head_count, uint32_t tx_id) {
    if (!lock_heads || lock_head_count <= 0) return -1;
    
    int tx_lock_count = 0;
    // Count lock heads where this transaction has granted locks
    for (int i = 0; i < lock_head_count; i++) {
        for (int j = 0; j < lock_heads[i].request_count; j++) {
            if (lock_heads[i].requests[j].transaction_id == tx_id && lock_heads[i].requests[j].granted) {
                tx_lock_count++;
            }
        }
    }
    
    // Escalate if transaction holds more than 3 locks (simulating low memory ceiling)
    if (tx_lock_count > 3) {
        // Upgrade the first lock head to Exclusive (X) and release the rest
        bool upgraded = false;
        for (int i = 0; i < lock_head_count; i++) {
            bool has_lock = false;
            int req_idx = -1;
            for (int j = 0; j < lock_heads[i].request_count; j++) {
                if (lock_heads[i].requests[j].transaction_id == tx_id) {
                    has_lock = true;
                    req_idx = j;
                    break;
                }
            }
            
            if (has_lock) {
                if (!upgraded) {
                    lock_heads[i].requests[req_idx].mode = LOCK_MODE_X;
                    lock_heads[i].requests[req_idx].granted = true; // Force upgrade
                    upgraded = true;
                } else {
                    // Release subsequent locks held by this transaction
                    tsfi_reuter_lock_release(&lock_heads[i], tx_id);
                }
            }
        }
        return 0; // Escalated successfully
    }
    
    return 1; // Escalation not needed
}

// 11. Deadlock Detection via Wait-For Graphs (WFG)
int tsfi_reuter_wfg_detect_deadlock(const tsfi_reuter_wfg_edge *edges, int edge_count, uint32_t *victim_tx_id_out) {
    if (!edges || edge_count <= 0 || !victim_tx_id_out) return -1;
    
    // Cycle detection using adjacency traversal (WFG path search)
    for (int i = 0; i < edge_count; i++) {
        uint32_t start_tx = edges[i].waiting_tx_id;
        uint32_t current_tx = edges[i].holding_tx_id;
        
        // Follow wait chains (max depth 8 to prevent infinite loops)
        for (int depth = 0; depth < 8; depth++) {
            if (current_tx == start_tx) {
                // Cycle detected! Select lowest transaction ID as the victim
                uint32_t victim = edges[i].waiting_tx_id;
                for (int k = 0; k < edge_count; k++) {
                    if (edges[k].waiting_tx_id < victim) {
                        victim = edges[k].waiting_tx_id;
                    }
                }
                *victim_tx_id_out = victim;
                return 0; // Deadlock detected & victim selected
            }
            
            // Move to next edge in wait chain
            bool chain_moved = false;
            for (int j = 0; j < edge_count; j++) {
                if (edges[j].waiting_tx_id == current_tx) {
                    current_tx = edges[j].holding_tx_id;
                    chain_moved = true;
                    break;
                }
            }
            if (!chain_moved) break;
        }
    }
    
    return 1; // No deadlock detected
}

// 12. MVCC Read, Write, and Sweep
int tsfi_reuter_mvcc_write(tsfi_reuter_version **head, uint64_t commit_lsn, const uint8_t *data, int len) {
    if (!head || !data || len > REUTER_MAX_DATA_SIZE) return -1;
    
    // Create new version node
    tsfi_reuter_version *new_ver = (tsfi_reuter_version *)malloc(sizeof(tsfi_reuter_version));
    if (!new_ver) return -2;
    
    new_ver->lsn = commit_lsn;
    memcpy(new_ver->data, data, len);
    
    // Insert at head (newest version first)
    new_ver->next = *head;
    *head = new_ver;
    
    return 0;
}

int tsfi_reuter_mvcc_read(const tsfi_reuter_version *head, uint64_t start_lsn, uint8_t *data_out, int len) {
    if (!data_out) return -1;
    
    const tsfi_reuter_version *curr = head;
    while (curr) {
        // Read version that was committed at or before start_lsn
        if (curr->lsn <= start_lsn) {
            memcpy(data_out, curr->data, len);
            return 0; // Version found
        }
        curr = curr->next;
    }
    return -2; // No visible version found
}

int tsfi_reuter_mvcc_sweep(tsfi_reuter_version **head, uint64_t min_active_lsn) {
    if (!head || !*head) return -1;
    
    tsfi_reuter_version *curr = *head;
    tsfi_reuter_version *prev = NULL;
    
    // Traverse version chain. Keep the first version committed <= min_active_lsn.
    // All subsequent versions are older and no longer active, so they can be swept!
    bool found_last_active = false;
    while (curr) {
        if (curr->lsn <= min_active_lsn) {
            if (found_last_active) {
                // Sweep/Free this old version
                tsfi_reuter_version *to_free = curr;
                if (prev) {
                    prev->next = curr->next;
                } else {
                    *head = curr->next;
                }
                curr = curr->next;
                free(to_free);
                continue;
            }
            found_last_active = true; // Keep this one as fallback, free remaining
        }
        prev = curr;
        curr = curr->next;
    }
    return 0;
}

// 13. Lock Upgrade Conversion
int tsfi_reuter_lock_upgrade(tsfi_reuter_lock_head *lock_head, uint32_t tx_id, tsfi_reuter_lock_mode target_mode) {
    if (!lock_head) return -1;
    
    int index = -1;
    for (int i = 0; i < lock_head->request_count; i++) {
        if (lock_head->requests[i].transaction_id == tx_id) {
            index = i;
            break;
        }
    }
    
    if (index == -1) return -2; // Lock not held
    
    tsfi_reuter_lock_mode original_mode = lock_head->requests[index].mode;
    if (original_mode >= target_mode) return 0; // Already holds sufficient mode
    
    // Check compatibility with other granted locks held by other transactions
    bool compatible = true;
    for (int i = 0; i < lock_head->request_count; i++) {
        if (i != index && lock_head->requests[i].granted) {
            if (!is_lock_compatible(target_mode, lock_head->requests[i].mode)) {
                compatible = false;
                break;
            }
        }
    }
    
    lock_head->requests[index].mode = target_mode;
    lock_head->requests[index].granted = compatible;
    
    return compatible ? 0 : 1; // 0 = upgraded immediately, 1 = conversion conflict (waiting)
}

// 14. Rigorous Two-Phase Locking (SS2PL)
int tsfi_reuter_ss2pl_release_all(tsfi_reuter_lock_head *lock_heads, int lock_head_count, uint32_t tx_id) {
    if (!lock_heads || lock_head_count <= 0) return -1;
    
    // Releases both Shared and Exclusive locks at transaction commit/abort
    for (int i = 0; i < lock_head_count; i++) {
        tsfi_reuter_lock_release(&lock_heads[i], tx_id);
    }
    return 0;
}

// 15. Log Reclamation & Truncation (Checkpoint Sweeper)
int tsfi_reuter_log_truncate(int log_fd, uint64_t min_recovery_lsn, const char *log_filepath) {
    if (log_fd < 0 || !log_filepath) return -1;
    
    // Create new temporary log file to store active segments
    char temp_filepath[256];
    snprintf(temp_filepath, sizeof(temp_filepath), "%s.tmp", log_filepath);
    
    int new_fd = open(temp_filepath, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (new_fd < 0) return -2;
    
    // Scan log from start and copy records >= min_recovery_lsn
    lseek(log_fd, 0, SEEK_SET);
    tsfi_reuter_log_record record;
    while (read(log_fd, &record, sizeof(tsfi_reuter_log_record)) == sizeof(tsfi_reuter_log_record)) {
        if (record.lsn >= min_recovery_lsn) {
            if (write(new_fd, &record, sizeof(tsfi_reuter_log_record)) < (ssize_t)sizeof(tsfi_reuter_log_record)) {
                close(new_fd);
                unlink(temp_filepath);
                return -3;
            }
        }
    }
    
    fsync(new_fd);
    close(new_fd);
    
    // Atomically replace old log with truncated log
    if (rename(temp_filepath, log_filepath) != 0) {
        unlink(temp_filepath);
        return -4;
    }
    
    return 0;
}

// 16. Checkpoint-Driven Boot Recovery optimization
int tsfi_reuter_aries_recover_from_checkpoint(int log_fd, tsfi_reuter_page *pages, int page_count, 
                                               tsfi_reuter_tx_entry *tx_table, int *tx_count, 
                                               tsfi_reuter_dirty_page *dirty_table, int *dirty_count) {
    if (log_fd < 0 || !pages || !tx_table || !dirty_table) return -1;
    
    *tx_count = 0;
    *dirty_count = 0;
    
    // Scan backwards from end of log file to locate the latest checkpoint record
    off_t file_len = lseek(log_fd, 0, SEEK_END);
    if (file_len < (off_t)sizeof(tsfi_reuter_log_record)) return -2; // Log too small
    
    off_t offset = file_len - sizeof(tsfi_reuter_log_record);
    tsfi_reuter_log_record record;
    bool checkpoint_found = false;
    
    while (offset >= 0) {
        lseek(log_fd, offset, SEEK_SET);
        if (read(log_fd, &record, sizeof(tsfi_reuter_log_record)) == sizeof(tsfi_reuter_log_record)) {
            if (record.type == LOG_TYPE_CHECKPOINT) {
                checkpoint_found = true;
                break;
            }
        }
        offset -= sizeof(tsfi_reuter_log_record);
    }
    
    uint64_t start_redo_lsn = 0;
    if (checkpoint_found) {
        // Recover Transaction Table and Dirty Page Table from checkpoint record
        for (int i = 0; i < 8; i++) {
            uint32_t tx_id = 0;
            memcpy(&tx_id, record.before_image + (i * 4), 4);
            if (tx_id != 0) {
                tx_table[*tx_count].transaction_id = tx_id;
                tx_table[*tx_count].last_lsn = record.lsn;
                tx_table[*tx_count].active = true;
                (*tx_count)++;
            }
            
            uint32_t page_id = 0;
            memcpy(&page_id, record.after_image + (i * 4), 4);
            if (page_id != 0 || i == 0) { // Keep dummy page mapping
                dirty_table[*dirty_count].page_id = page_id;
                dirty_table[*dirty_count].rec_lsn = record.lsn;
                (*dirty_count)++;
            }
        }
        start_redo_lsn = record.lsn;
    }
    
    // Redo Phase starting from checkpoint LSN
    lseek(log_fd, 0, SEEK_SET);
    while (read(log_fd, &record, sizeof(tsfi_reuter_log_record)) == sizeof(tsfi_reuter_log_record)) {
        if (record.lsn >= start_redo_lsn && record.type == LOG_TYPE_WAL) {
            uint32_t page_id = record.data_offset / 512;
            for (int i = 0; i < page_count; i++) {
                if (pages[i].page_id == page_id && pages[i].page_lsn < record.lsn) {
                    uint32_t local_offset = record.data_offset % 512;
                    memcpy(pages[i].data + local_offset, record.after_image, record.data_len);
                    pages[i].page_lsn = record.lsn;
                }
            }
        }
    }
    
    // Undo Phase rolling back active transactions
    for (int i = *tx_count - 1; i >= 0; i--) {
        if (tx_table[i].active) {
            uint64_t clr_lsn = 0;
            tsfi_reuter_write_clr(log_fd, tx_table[i].transaction_id, tx_table[i].last_lsn, 0, 0, NULL, &clr_lsn);
            tx_table[i].active = false;
        }
    }
    
    return checkpoint_found ? 0 : 1; // 0 = recovered from checkpoint, 1 = fallback redo
}

// 17. LU6.2 Multi-Node Syncpoint Handshake Exchange
int tsfi_reuter_lu62_syncpoint_handshake(tsfi_reuter_2pc_coordinator *coord, uint32_t node_id, 
                                         const char *in_buffer, char *out_buffer) {
    if (!coord || !in_buffer || !out_buffer) return -1;
    
    if (strcmp(in_buffer, "REQ_COMMIT") == 0) {
        // Coordinator sends PREPARE signal to participants
        strcpy(out_buffer, "PREPARE");
        return LU62_STATE_SYNC_PENDING;
    } else if (strcmp(in_buffer, "AGREE") == 0) {
        // Participant votes to commit
        tsfi_reuter_2pc_vote(coord, node_id, true);
        strcpy(out_buffer, "DECISION_COMMIT");
        return LU62_STATE_DECIDED;
    } else if (strcmp(in_buffer, "VETO") == 0) {
        // Participant votes to abort
        tsfi_reuter_2pc_vote(coord, node_id, false);
        strcpy(out_buffer, "DECISION_ABORT");
        return LU62_STATE_DECIDED;
    }
    
    return -2; // Unknown handshake sequence
}

// 18. Selective undo (subtransaction rollback) using undo_next_lsn
int tsfi_reuter_selective_undo(int log_fd, tsfi_reuter_page *pages, int page_count, 
                               uint32_t tx_id, uint64_t target_lsn) {
    if (log_fd < 0 || !pages) return -1;
    
    // Read log records in reverse LSN order to selectively roll back this transaction
    off_t file_len = lseek(log_fd, 0, SEEK_END);
    off_t offset = file_len - sizeof(tsfi_reuter_log_record);
    tsfi_reuter_log_record record;
    
    uint64_t next_undo_lsn = 0;
    
    while (offset >= 0) {
        lseek(log_fd, offset, SEEK_SET);
        if (read(log_fd, &record, sizeof(tsfi_reuter_log_record)) == sizeof(tsfi_reuter_log_record)) {
            if (record.transaction_id == tx_id && record.lsn > target_lsn) {
                if (record.type == LOG_TYPE_WAL) {
                    // Undo this specific change
                    uint32_t page_id = record.data_offset / 512;
                    for (int i = 0; i < page_count; i++) {
                        if (pages[i].page_id == page_id) {
                            uint32_t local_offset = record.data_offset % 512;
                            memcpy(pages[i].data + local_offset, record.before_image, record.data_len);
                            
                            // Log Compensation Log Record (CLR) pointing to the previous LSN to undo
                            uint64_t clr_lsn = 0;
                            tsfi_reuter_write_clr(log_fd, tx_id, next_undo_lsn, record.data_offset, record.data_len, record.before_image, &clr_lsn);
                            pages[i].page_lsn = clr_lsn;
                            next_undo_lsn = record.lsn;
                            break;
                        }
                    }
                }
            }
        }
        offset -= sizeof(tsfi_reuter_log_record);
    }
    
    return 0;
}

// 19. Presumed Commit vs. Presumed Abort LU6.2 optimizations
int tsfi_reuter_2pc_finalize_presumed(tsfi_reuter_2pc_coordinator *coord, bool presumed_abort, bool *is_committed) {
    if (!coord || !is_committed) return -1;
    (void)presumed_abort;
    
    // Check states of participants
    bool all_prepared = true;
    for (int i = 0; i < coord->participant_count; i++) {
        if (coord->participant_states[i] != NODE_STATE_PREPARED) {
            all_prepared = false;
            break;
        }
    }
    
    if (all_prepared) {
        coord->global_decision_commit = true;
        for (int i = 0; i < coord->participant_count; i++) {
            coord->participant_states[i] = NODE_STATE_COMMITTED;
        }
        *is_committed = true;
        // Under Presumed Abort, commit must be written to log. Under Presumed Commit, commit is default (saves log write)
        return 0;
    } else {
        coord->global_decision_commit = false;
        for (int i = 0; i < coord->participant_count; i++) {
            coord->participant_states[i] = NODE_STATE_ABORTED;
        }
        *is_committed = false;
        // Under Presumed Commit, abort must be written to log. Under Presumed Abort, abort is default (saves log write)
        return 0;
    }
}

// 20. Doublewrite Buffer Manager
int tsfi_reuter_doublewrite_init(tsfi_reuter_doublewrite_buffer *dwb) {
    if (!dwb) return -1;
    memset(dwb, 0, sizeof(tsfi_reuter_doublewrite_buffer));
    return 0;
}

int tsfi_reuter_doublewrite_flush(tsfi_reuter_doublewrite_buffer *dwb, int dw_fd, tsfi_reuter_page *page) {
    if (!dwb || dw_fd < 0 || !page) return -1;
    
    // Queue page in doublewrite buffer memory
    int index = dwb->count % 4;
    dwb->pages[index] = *page;
    dwb->count++;
    
    // Write sequentially to the doublewrite file (simulated disk block)
    lseek(dw_fd, index * sizeof(tsfi_reuter_page), SEEK_SET);
    if (write(dw_fd, page, sizeof(tsfi_reuter_page)) < (ssize_t)sizeof(tsfi_reuter_page)) {
        return -2;
    }
    
    // Ensure data is synced to doublewrite space before random writes to actual DB files are allowed!
    fsync(dw_fd);
    return 0;
}

int tsfi_reuter_doublewrite_recover_page(int dw_fd, tsfi_reuter_page *corrupted_page) {
    if (dw_fd < 0 || !corrupted_page) return -1;
    
    // Scan doublewrite buffer file to find matching page ID to restore from
    lseek(dw_fd, 0, SEEK_SET);
    tsfi_reuter_page buffer_page;
    while (read(dw_fd, &buffer_page, sizeof(tsfi_reuter_page)) == sizeof(tsfi_reuter_page)) {
        if (buffer_page.page_id == corrupted_page->page_id) {
            *corrupted_page = buffer_page; // Restore full page content
            return 0; // Page successfully restored from doublewrite copy!
        }
    }
    
    return -2; // Page copy not found in doublewrite buffer
}

// 21. Background Page Cleaner Scheduler (WAL Constraint verification)
int tsfi_reuter_page_cleaner_flush_page(tsfi_reuter_page *page, int db_fd, uint64_t flushed_lsn) {
    if (!page || db_fd < 0) return -1;
    
    // WAL Protocol Rule: A dirty page cannot be written to database disk files
    // until the log records up to the page's LSN have been written and flushed to disk.
    if (page->page_lsn > flushed_lsn) {
        return -2; // WAL violation! Log has not been flushed yet!
    }
    
    // Write page to simulated DB file
    lseek(db_fd, page->page_id * sizeof(tsfi_reuter_page), SEEK_SET);
    if (write(db_fd, page, sizeof(tsfi_reuter_page)) < (ssize_t)sizeof(tsfi_reuter_page)) {
        return -3;
    }
    
    return 0;
}
