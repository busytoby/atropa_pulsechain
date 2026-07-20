#ifndef TSFI_REUTER_TX_H
#define TSFI_REUTER_TX_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define REUTER_MAX_DATA_SIZE 64
#define REUTER_MAX_NODES 8

// Andreas Reuter Log Record (WAL)
typedef struct {
    uint64_t lsn;
    uint32_t transaction_id;
    uint32_t data_offset;
    uint32_t data_len;
    uint8_t before_image[REUTER_MAX_DATA_SIZE];
    uint8_t after_image[REUTER_MAX_DATA_SIZE];
} tsfi_reuter_log_record;

// Andreas Reuter Compensation Log Record (CLR)
typedef struct {
    uint64_t lsn;
    uint64_t undo_next_lsn;
    uint32_t transaction_id;
    uint32_t data_offset;
    uint32_t data_len;
    uint8_t compensation_image[REUTER_MAX_DATA_SIZE];
} tsfi_reuter_clr_record;

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

// WAL and Recovery Manager APIs
void tsfi_reuter_init(void);
uint64_t tsfi_reuter_get_global_lsn(void);

int tsfi_reuter_write_wal(int log_fd, uint32_t tx_id, uint32_t offset, uint32_t len, 
                          const uint8_t *before, const uint8_t *after, uint64_t *assigned_lsn);

int tsfi_reuter_write_clr(int log_fd, uint32_t tx_id, uint64_t undo_next_lsn, uint32_t offset, 
                          uint32_t len, const uint8_t *comp_data, uint64_t *assigned_lsn);

// LU6.2 2PC APIs
void tsfi_reuter_2pc_init(tsfi_reuter_2pc_coordinator *coord, uint32_t tx_id);
int tsfi_reuter_2pc_add_participant(tsfi_reuter_2pc_coordinator *coord, uint32_t node_id);
int tsfi_reuter_2pc_vote(tsfi_reuter_2pc_coordinator *coord, uint32_t node_id, bool vote_commit);
int tsfi_reuter_2pc_finalize(tsfi_reuter_2pc_coordinator *coord, bool *is_committed);

#endif // TSFI_REUTER_TX_H
