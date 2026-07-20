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

#endif // TSFI_AUTODIN_SAGE_H
