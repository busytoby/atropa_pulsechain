#ifndef TSFI_AUTODIN_SAGE_H
#define TSFI_AUTODIN_SAGE_H

#include <stdint.h>
#include <stdbool.h>
#include "tsfi_gray_tx.h"

// 1. AUTODIN Store-and-Forward Latching Relay Structures
typedef struct {
    int32_t set_flux;      // Magnetic flux on SET coil
    int32_t reset_flux;    // Magnetic flux on RESET coil
    bool contacts_closed;   // Physical connection status
} tsfi_autodin_relay;

// 2. SAGE PLL (Phase-Locked Loop) Fabric and CICS I/O System
typedef struct {
    int32_t reference_phase;
    int32_t feedback_phase;
    int32_t error_voltage;
    int32_t frequency_offset;
} tsfi_sage_pll_fabric;

typedef struct {
    uint32_t tx_id;
    uint32_t queue_id;
    uint8_t buffer[256];
    int len;
} tsfi_sage_cics_io;

// 3. AUTODIN Transaction and Heartbeat Manager
typedef struct {
    uint32_t active_tx_id;
    uint64_t last_heartbeat;
    bool system_unstable;
    tsfi_sage_pll_fabric pll;
} tsfi_autodin_manager;

// 4. WinchesterMQ SCSI Loopback Handshake (Auncient Hardware Routing)
typedef struct {
    uint8_t keycode_reg;   // SCSI keycode register (32 for D/d, 30 for A/a)
    bool req_line;         // SCSI REQ line state
    bool ack_line;         // SCSI ACK line state
    int handshake_step;    // State machine step
} tsfi_winchester_scsi;

// APIs
void tsfi_autodin_relay_init(tsfi_autodin_relay *relay);
int tsfi_autodin_relay_latch(tsfi_autodin_relay *relay, int32_t set_current, int32_t reset_current);

int tsfi_sage_pll_sync(tsfi_sage_pll_fabric *pll, int32_t external_phase);
int tsfi_sage_cics_io_route(tsfi_sage_cics_io *io, tsfi_autodin_manager *mgr);

int tsfi_winchester_scsi_handshake(tsfi_winchester_scsi *scsi, int loopback_socket_fd, uint8_t input_keycode);

// AUTODIN Precedence Levels
typedef enum {
    AUTODIN_PRECEDENCE_ROUTINE,
    AUTODIN_PRECEDENCE_PRIORITY,
    AUTODIN_PRECEDENCE_IMMEDIATE,
    AUTODIN_PRECEDENCE_FLASH
} tsfi_autodin_precedence;

// SAGE Duplex Processor State
typedef struct {
    uint32_t active_cpu_id;
    uint32_t standby_cpu_id;
    uint64_t last_sync_time;
    bool standby_active;
} tsfi_sage_duplex;

// Expanded SAGE / AUTODIN APIs
int tsfi_sage_filter_tracks(int32_t *x_out, int32_t *y_out, const int32_t *stations_x, const int32_t *stations_y, int count);
int tsfi_sage_duplex_sync(tsfi_sage_duplex *duplex, bool active_alive);
int tsfi_autodin_schedule_message(tsfi_autodin_precedence precedence, const char *msg, char *out_queue_buf);

// SAGE Magnetic Drum Memory buffer
typedef struct {
    uint8_t tracks[8][64];
    uint32_t head_position;
} tsfi_sage_drum;

// AUTODIN routing indicator mapping
typedef struct {
    char routing_indicator[8];
    uint32_t output_channel;
} tsfi_autodin_route;

int tsfi_sage_drum_write(tsfi_sage_drum *drum, int track_idx, int sector_idx, const uint8_t *data, int len);
int tsfi_sage_drum_read(tsfi_sage_drum *drum, int track_idx, int sector_idx, uint8_t *data_out, int len);
int tsfi_autodin_find_route(const tsfi_autodin_route *table, int table_size, const char *ri, uint32_t *channel_out);

// SAGE Marginal checking unit (Vacuum tubes voltage variations)
typedef struct {
    int32_t baseline_voltage_mv;
    int32_t applied_voltage_mv;
    int32_t tube_emission_percent;
} tsfi_sage_marginal_unit;

// AUTODIN Preemption control channel
typedef struct {
    bool channel_busy;
    tsfi_autodin_precedence current_precedence;
    uint32_t suspended_tx_id;
} tsfi_autodin_preempt_channel;

int tsfi_sage_marginal_check(tsfi_sage_marginal_unit *unit, int32_t voltage_offset_mv);
int tsfi_autodin_preempt_check(tsfi_autodin_preempt_channel *chan, uint32_t new_tx_id, tsfi_autodin_precedence new_prec, bool *action_preempt, bool *action_reject);

// SAGE Light Gun operator input mapping
typedef struct {
    int32_t target_x;
    int32_t target_y;
    bool trigger_pulled;
} tsfi_sage_light_gun;

// AUTODIN Store-and-Forward Tape Journal
typedef struct {
    int journal_fd;
    uint64_t last_journal_lsn;
} tsfi_autodin_journal;

int tsfi_sage_light_gun_select(const tsfi_sage_light_gun *gun, const int32_t *track_x, const int32_t *track_y, int track_count, int *selected_track_idx);
int tsfi_autodin_journal_write(tsfi_autodin_journal *j, uint64_t lsn, const char *msg);

// SAGE Console Vector Drawing Definition
typedef struct {
    int32_t start_x;
    int32_t start_y;
    int32_t end_x;
    int32_t end_y;
    uint32_t intensity_reg;
} tsfi_sage_vector;

int tsfi_sage_generate_vector(tsfi_sage_vector *vec, int32_t sx, int32_t sy, int32_t ex, int32_t ey, uint32_t intensity);

// SAGE & AUTODIN Bridges to Gray & Reuter Transactions
int tsfi_sage_duplex_reuter_sync(tsfi_sage_duplex *duplex, const tsfi_reuter_tx_entry *active_table, int active_count, tsfi_reuter_tx_entry *standby_table, int *standby_count);
int tsfi_autodin_preempt_gray_locks(tsfi_autodin_preempt_channel *chan, tsfi_reuter_lock_head *locks, int lock_count, tsfi_gray_consistency_degree degree);

int tsfi_sage_duplex_checkpoint_recover(tsfi_sage_duplex *duplex, int log_fd, tsfi_reuter_page *pages, int page_count, tsfi_reuter_tx_entry *tx_table, int *tx_count, tsfi_reuter_dirty_page *dirty_table, int *dirty_count);
int tsfi_autodin_preempt_cascade_abort(tsfi_autodin_preempt_channel *chan, tsfi_gray_abort_tracker *tracker, uint32_t *cascaded_aborts_out, int *cascade_count);

int tsfi_sage_duplex_lu62_commit(tsfi_sage_duplex *duplex, tsfi_reuter_2pc_coordinator *coord, uint32_t standby_node_id);
int tsfi_autodin_preempt_evict_cache(tsfi_autodin_preempt_channel *chan, tsfi_gray_cache_manager *cm, uint64_t current_time, uint32_t *evicted_page_id);

int tsfi_sage_duplex_group_commit(tsfi_sage_duplex *duplex, tsfi_reuter_group_commit *gc, uint32_t active_tx_id, uint32_t standby_tx_id);
int tsfi_autodin_preempt_mvcc_sweep(tsfi_autodin_preempt_channel *chan, tsfi_reuter_version **version_head, uint64_t min_active_lsn);

int tsfi_sage_vulkan_project(int32_t track_x, int32_t track_y, float *ndc_x, float *ndc_y);

// SAGE Keyboard / Mouse Event Audited under CICS
typedef struct {
    uint32_t event_type; // 1 = Keyboard, 2 = Mouse
    uint32_t key_code;
    int32_t mouse_x;
    int32_t mouse_y;
} tsfi_sage_cics_event;

int tsfi_sage_cics_audit_event(tsfi_reuter_group_commit *gc, uint32_t tx_id, const tsfi_sage_cics_event *event);
int tsfi_sage_light_gun_audit(tsfi_reuter_group_commit *gc, uint32_t tx_id, const tsfi_sage_light_gun *gun, const int32_t *track_x, const int32_t *track_y, int track_count, int *selected_track_idx);
int tsfi_sage_cics_event_parity_verify(const tsfi_sage_cics_event *event, uint8_t parity_bit);
int tsfi_winchester_scsi_parity_verify(const tsfi_winchester_scsi *scsi, uint8_t parity_bit);

#endif // TSFI_AUTODIN_SAGE_H
