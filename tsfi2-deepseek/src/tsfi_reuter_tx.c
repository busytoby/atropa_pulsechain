#include "tsfi_reuter_tx.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

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
    int rc = tsfi_reuter_write_wal(log_fd, tx_id, offset, len, before, new_data, &assigned_lsn);
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
    
    tsfi_reuter_checkpoint chk;
    memset(&chk, 0, sizeof(tsfi_reuter_checkpoint));
    
    chk.checkpoint_lsn = ++global_lsn;
    chk.active_tx_count = tx_count < 16 ? tx_count : 16;
    for (int i = 0; i < chk.active_tx_count; i++) {
        chk.active_tx_ids[i] = tx_table[i].transaction_id;
        chk.active_tx_last_lsns[i] = tx_table[i].last_lsn;
    }
    
    chk.dirty_page_count = dirty_count < 16 ? dirty_count : 16;
    for (int i = 0; i < chk.dirty_page_count; i++) {
        chk.dirty_page_ids[i] = dirty_table[i].page_id;
        chk.dirty_page_rec_lsns[i] = dirty_table[i].rec_lsn;
    }
    
    // Write checkpoint record to stable storage
    ssize_t bytes_written = write(log_fd, &chk, sizeof(tsfi_reuter_checkpoint));
    if (bytes_written < (ssize_t)sizeof(tsfi_reuter_checkpoint)) {
        return -2;
    }
    
    // Fuzzy checkpoint does NOT block other writers, just flushes metadata
    fsync(log_fd);
    
    if (chk_lsn) {
        *chk_lsn = chk.checkpoint_lsn;
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
