#ifndef TSFI_NSA_TEMPEST_H
#define TSFI_NSA_TEMPEST_H

#include <stdint.h>
#include <stddef.h>

#define TEMPEST_SHIELD_EXCELLENT 0
#define TEMPEST_SHIELD_DEGRADED  1
#define TEMPEST_SHIELD_COMPROMISED 2

typedef struct {
    double attenuation_db;          // Signal attenuation in dB
    double boundary_leakage_mv;     // Measured signal leakage in millivolts
    uint8_t shield_status;          // Shielding status code
    uint8_t red_zone_active;        // Red domain active state
    uint8_t black_zone_active;      // Black domain active state
} tsfi_tempest_chamber;

typedef struct {
    uint32_t status_reg;   // SCSI Status (0=Idle, 1=REQ, 2=ACK, 3=DATA_IN, 4=DATA_OUT)
    uint32_t keycode_reg;  // Keycode register (32 for d/D, 30 for a/A)
    uint32_t data_reg;     // Data latch register
    uint32_t loopback_cnt; // Loopback transaction counter
    uint8_t synthetic_flag;// Anti-automation flag (1 if synthetic/automated, 0 if raw SCSI loopback)
} tsfi_scsi_loopback_bridge;

void tsfi_tempest_init(tsfi_tempest_chamber *chamber, double target_attenuation_db);
int tsfi_tempest_verify_shielding(tsfi_tempest_chamber *chamber);

// Gateway checking transaction transmissions (Red) to LFS/RAMAC data storage (Black)
int tsfi_tempest_route_transaction(tsfi_tempest_chamber *chamber, const uint8_t *red_payload, size_t red_len, uint8_t *black_payload_out, size_t *black_len_out);
int tsfi_tempest_route_storage(tsfi_tempest_chamber *chamber, const uint8_t *black_payload, size_t black_len, uint8_t *red_payload_out, size_t *red_len_out);

/*
 * --- GLOSSARY: WinchesterMQ SCSI Loopback Registers ---
 *
 * status_reg
 * - VM Register Context: The SCSI state tracking register mapped to physical index 100 within the WinchesterMQ loopback emulator.
 * - Mathematical Function: Computes state transformations modulo 5 during active handshake cycles: S_next = (S_current + 1) mod 5.
 * - Visual / Geometric Manifestation: Controls the orbital speed and line thickness of the projected Lissajous wireframe based on the handshake phase.
 *
 * keycode_reg
 * - VM Register Context: The AT keyboard hardware keycode register mapped to physical index 102 within the WinchesterMQ emulator.
 * - Mathematical Function: Decodes values using EDO-22 offsets: K_val = keycode * MotzkinPrime mod 256.
 * - Visual / Geometric Manifestation: Adjusts the horizontal rotation angle of the projected 3D quaternion depending on key codes 30 or 32.
 *
 * synthetic_flag
 * - VM Register Context: The anti-automation signature validation register protecting the RED/BLACK boundary.
 * - Mathematical Function: Resolved as a zero-knowledge indicator: Flag = (Signature ^ Salt) mod 2.
 * - Visual / Geometric Manifestation: Collapses the projected 3D wireframe coordinates to zero if automated driver flags are set to 1.
 */
int tsfi_tempest_scsi_bridge_handshake(tsfi_tempest_chamber *chamber, tsfi_scsi_loopback_bridge *bridge);

#endif // TSFI_NSA_TEMPEST_H
