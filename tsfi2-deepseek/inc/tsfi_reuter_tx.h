#ifndef TSFI_REUTER_TX_H
#define TSFI_REUTER_TX_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define REUTER_MAX_DATA_SIZE 64
#define REUTER_MAX_NODES 8

typedef enum {
    LOG_TYPE_WAL,
    LOG_TYPE_CLR,
    LOG_TYPE_CHECKPOINT
} tsfi_reuter_log_type;

// Andreas Reuter Log Record (Unified WAL / CLR format)
typedef struct {
    uint64_t lsn;
    uint32_t transaction_id;
    uint32_t type;          // tsfi_reuter_log_type
    uint64_t undo_next_lsn; // Pointer to next LSN to undo (CLR only)
    uint32_t data_offset;
    uint32_t data_len;
    uint8_t before_image[REUTER_MAX_DATA_SIZE];
    uint8_t after_image[REUTER_MAX_DATA_SIZE];
} tsfi_reuter_log_record;

// Two-Phase Commit Node States
typedef enum {
    NODE_STATE_INIT,
    NODE_STATE_PREPARED,
    NODE_STATE_COMMITTED,
    NODE_STATE_ABORTED
} tsfi_reuter_node_state;

// LU6.2 Two-Phase Commit Coordinator State
typedef struct {
    uint32_t transaction_id;
    int participant_count;
    uint32_t participant_ids[REUTER_MAX_NODES];
    tsfi_reuter_node_state participant_states[REUTER_MAX_NODES];
    bool global_decision_commit;
} tsfi_reuter_2pc_coordinator;

// Lock Modes for Hierarchical Locking
typedef enum {
    LOCK_MODE_IS,  // Intent Shared
    LOCK_MODE_IX,  // Intent Exclusive
    LOCK_MODE_S,   // Shared
    LOCK_MODE_SIX, // Shared with Intent Exclusive
    LOCK_MODE_X    // Exclusive
} tsfi_reuter_lock_mode;

typedef struct {
    uint32_t transaction_id;
    tsfi_reuter_lock_mode mode;
    bool granted;
} tsfi_reuter_lock_request;

typedef struct {
    uint32_t resource_id;
    int request_count;
    tsfi_reuter_lock_request requests[8];
} tsfi_reuter_lock_head;

// Physiological page model with PageLSN in page header
typedef struct {
    uint64_t page_lsn;
    uint32_t page_id;
    uint32_t free_offset;
    uint8_t data[512];
} tsfi_reuter_page;

// ARIES Transaction Table Entry
typedef struct {
    uint32_t transaction_id;
    uint64_t last_lsn;
    bool active;
} tsfi_reuter_tx_entry;

// ARIES Dirty Page Table Entry
typedef struct {
    uint32_t page_id;
    uint64_t rec_lsn;
} tsfi_reuter_dirty_page;

// Savepoints and Nested Transactions structures
typedef struct {
    char name[32];
    uint64_t savepoint_lsn;
} tsfi_reuter_savepoint;

typedef struct {
    uint32_t transaction_id;
    uint32_t parent_id;
    int savepoint_count;
    tsfi_reuter_savepoint savepoints[8];
} tsfi_reuter_tx_context;

// Fuzzy Checkpoint Metadata
typedef struct {
    uint64_t checkpoint_lsn;
    int active_tx_count;
    uint32_t active_tx_ids[16];
    uint64_t active_tx_last_lsns[16];
    int dirty_page_count;
    uint32_t dirty_page_ids[16];
    uint64_t dirty_page_rec_lsns[16];
} tsfi_reuter_checkpoint;

// Group Commit Coordinator
typedef struct {
    int log_fd;
    int queued_count;
    tsfi_reuter_log_record queued_records[16];
    uint64_t *assigned_lsns[16];
} tsfi_reuter_group_commit;

// WAL and Recovery Manager APIs
void tsfi_reuter_init(void);
uint64_t tsfi_reuter_get_global_lsn(void);

int tsfi_reuter_write_wal(int log_fd, uint32_t tx_id, uint32_t offset, uint32_t len, 
                          const uint8_t *before, const uint8_t *after, uint64_t *assigned_lsn);

int tsfi_reuter_write_clr(int log_fd, uint32_t tx_id, uint64_t undo_next_lsn, uint32_t offset, 
                          uint32_t len, const uint8_t *comp_data, uint64_t *assigned_lsn);

// Physiological Page API
int tsfi_reuter_page_write(tsfi_reuter_page *page, int log_fd, uint32_t tx_id, uint32_t offset, 
                           uint32_t len, const uint8_t *new_data);

// ARIES 3-Phase Crash Recovery API
int tsfi_reuter_aries_recover(int log_fd, tsfi_reuter_page *pages, int page_count, 
                              tsfi_reuter_tx_entry *tx_table, int *tx_count, 
                              tsfi_reuter_dirty_page *dirty_table, int *dirty_count);

// Lock Manager API
int tsfi_reuter_lock_acquire(tsfi_reuter_lock_head *lock_head, uint32_t tx_id, tsfi_reuter_lock_mode mode);
int tsfi_reuter_lock_release(tsfi_reuter_lock_head *lock_head, uint32_t tx_id);

// LU6.2 2PC APIs
void tsfi_reuter_2pc_init(tsfi_reuter_2pc_coordinator *coord, uint32_t tx_id);
int tsfi_reuter_2pc_add_participant(tsfi_reuter_2pc_coordinator *coord, uint32_t node_id);
int tsfi_reuter_2pc_vote(tsfi_reuter_2pc_coordinator *coord, uint32_t node_id, bool vote_commit);
int tsfi_reuter_2pc_finalize(tsfi_reuter_2pc_coordinator *coord, bool *is_committed);

// MVCC Version Chain Node
typedef struct tsfi_reuter_version {
    uint64_t lsn;
    uint8_t data[REUTER_MAX_DATA_SIZE];
    struct tsfi_reuter_version *next;
} tsfi_reuter_version;

// WFG (Wait-For Graph) Edge
typedef struct {
    uint32_t waiting_tx_id;
    uint32_t holding_tx_id;
} tsfi_reuter_wfg_edge;

// Savepoint & Nested Transaction APIs
int tsfi_reuter_savepoint_create(tsfi_reuter_tx_context *ctx, const char *name, uint64_t current_lsn);
uint64_t tsfi_reuter_savepoint_rollback(tsfi_reuter_tx_context *ctx, const char *name);

// Fuzzy Checkpoint API
int tsfi_reuter_write_checkpoint(int log_fd, const tsfi_reuter_tx_entry *tx_table, int tx_count, 
                                 const tsfi_reuter_dirty_page *dirty_table, int dirty_count, 
                                 uint64_t *chk_lsn);

// Group Commit APIs
void tsfi_reuter_group_commit_init(tsfi_reuter_group_commit *gc, int log_fd);
int tsfi_reuter_group_commit_queue(tsfi_reuter_group_commit *gc, uint32_t tx_id, uint32_t offset, 
                                    uint32_t len, const uint8_t *before, const uint8_t *after, 
                                    uint64_t *assigned_lsn);
int tsfi_reuter_group_commit_flush(tsfi_reuter_group_commit *gc);

// Lock Escalation API
int tsfi_reuter_lock_escalate(tsfi_reuter_lock_head *lock_heads, int lock_head_count, uint32_t tx_id);

// Deadlock Detection API
int tsfi_reuter_wfg_detect_deadlock(const tsfi_reuter_wfg_edge *edges, int edge_count, uint32_t *victim_tx_id_out);

// MVCC APIs
int tsfi_reuter_mvcc_write(tsfi_reuter_version **head, uint64_t commit_lsn, const uint8_t *data, int len);
int tsfi_reuter_mvcc_read(const tsfi_reuter_version *head, uint64_t start_lsn, uint8_t *data_out, int len);
int tsfi_reuter_mvcc_sweep(tsfi_reuter_version **head, uint64_t min_active_lsn);

// Lock Upgrade Conversion API
int tsfi_reuter_lock_upgrade(tsfi_reuter_lock_head *lock_head, uint32_t tx_id, tsfi_reuter_lock_mode target_mode);

// Rigorous 2PL (SS2PL) API
int tsfi_reuter_ss2pl_release_all(tsfi_reuter_lock_head *lock_heads, int lock_head_count, uint32_t tx_id);

// Log Truncation Sweeper API
int tsfi_reuter_log_truncate(int log_fd, uint64_t min_recovery_lsn, const char *log_filepath);

// LU6.2 Conversational Syncpoint States
typedef enum {
    LU62_STATE_SEND,
    LU62_STATE_RECEIVE,
    LU62_STATE_SYNC_PENDING,
    LU62_STATE_DECIDED
} tsfi_reuter_lu62_state;

// Checkpoint-Driven Boot Recovery API
int tsfi_reuter_aries_recover_from_checkpoint(int log_fd, tsfi_reuter_page *pages, int page_count, 
                                               tsfi_reuter_tx_entry *tx_table, int *tx_count, 
                                               tsfi_reuter_dirty_page *dirty_table, int *dirty_count);

// LU6.2 Multi-Node Syncpoint Handshake Exchange API
int tsfi_reuter_lu62_syncpoint_handshake(tsfi_reuter_2pc_coordinator *coord, uint32_t node_id, 
                                         const char *in_buffer, char *out_buffer);

// Selective subtransaction undo via undo_next_lsn
int tsfi_reuter_selective_undo(int log_fd, tsfi_reuter_page *pages, int page_count, 
                               uint32_t tx_id, uint64_t target_lsn);

// Presumed Commit vs. Presumed Abort 2PC API
int tsfi_reuter_2pc_finalize_presumed(tsfi_reuter_2pc_coordinator *coord, bool presumed_abort, bool *is_committed);

// Doublewrite Buffer Structure and APIs
typedef struct {
    tsfi_reuter_page pages[4];
    int count;
} tsfi_reuter_doublewrite_buffer;

int tsfi_reuter_doublewrite_init(tsfi_reuter_doublewrite_buffer *dwb);
int tsfi_reuter_doublewrite_flush(tsfi_reuter_doublewrite_buffer *dwb, int dw_fd, tsfi_reuter_page *page);
int tsfi_reuter_doublewrite_recover_page(int dw_fd, tsfi_reuter_page *corrupted_page);

// Background Page Cleaner Scheduler API
int tsfi_reuter_page_cleaner_flush_page(tsfi_reuter_page *page, int db_fd, uint64_t flushed_lsn);

#endif // TSFI_REUTER_TX_H
