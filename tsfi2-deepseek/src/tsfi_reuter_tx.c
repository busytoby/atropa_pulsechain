#include "tsfi_reuter_tx.h"
#include <string.h>
#include <unistd.h>

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
    
    tsfi_reuter_clr_record record;
    memset(&record, 0, sizeof(tsfi_reuter_clr_record));
    
    record.lsn = ++global_lsn;
    record.undo_next_lsn = undo_next_lsn;
    record.transaction_id = tx_id;
    record.data_offset = offset;
    record.data_len = len;
    
    if (comp_data) {
        memcpy(record.compensation_image, comp_data, len);
    }
    
    // Write Compensation Log Record to stable storage
    ssize_t bytes_written = write(log_fd, &record, sizeof(tsfi_reuter_clr_record));
    if (bytes_written < (ssize_t)sizeof(tsfi_reuter_clr_record)) {
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
