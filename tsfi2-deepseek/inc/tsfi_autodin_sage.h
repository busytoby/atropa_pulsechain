#ifndef TSFI_AUTODIN_SAGE_H
#define TSFI_AUTODIN_SAGE_H

#include <stdint.h>
#include <stdbool.h>

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

#endif // TSFI_AUTODIN_SAGE_H
