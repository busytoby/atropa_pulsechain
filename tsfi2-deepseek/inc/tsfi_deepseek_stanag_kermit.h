#ifndef TSFI_DEEPSEEK_STANAG_KERMIT_H
#define TSFI_DEEPSEEK_STANAG_KERMIT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define STANAG_KERMIT_MAX_FRAME_LEN     1024
#define STANAG_KERMIT_MARK_START        0x01 // SOH / Ctrl-A
#define STANAG_SAP_CORDIS_TELEMETRY     0x2A // SAP 42: Agent Telemetry

// ============================================================================
// 1. Coaxial Kermit Packet Encoder & Checksum Verifier
// ============================================================================

typedef enum {
    KERMIT_TYPE_DATA        = 'D',
    KERMIT_TYPE_ACK         = 'Y',
    KERMIT_TYPE_NAK         = 'N',
    KERMIT_TYPE_TELEMETRY   = 'T',
    KERMIT_TYPE_BREAK       = 'B'
} tsfi_kermit_packet_type_t;

typedef struct {
    uint8_t mark;               // 0x01 (SOH)
    uint8_t length;             // Length of remaining fields + 32 (ASCII bias)
    uint8_t seq;                // Packet sequence number (0-63 + 32)
    uint8_t type;               // tsfi_kermit_packet_type_t
    uint8_t data[STANAG_KERMIT_MAX_FRAME_LEN];
    size_t  data_len;
    uint8_t checksum;           // 6-bit Kermit arithmetic checksum
} tsfi_kermit_packet_t;

bool tsfi_kermit_packet_init(tsfi_kermit_packet_t *pkt, uint8_t seq, tsfi_kermit_packet_type_t type);
bool tsfi_kermit_encode_frame(
    const tsfi_kermit_packet_t *pkt,
    uint8_t *raw_out,
    size_t *raw_out_len
);
bool tsfi_kermit_decode_frame(
    const uint8_t *raw_in,
    size_t raw_len,
    tsfi_kermit_packet_t *pkt_out
);

// ============================================================================
// 2. STANAG 5066 / 5516 Coaxial Link-Layer Encapsulator
// ============================================================================

typedef struct {
    uint8_t  sap_address;       // Service Access Point
    uint8_t  priority;          // Link QoS Priority (0-15)
    uint16_t node_address;      // 16-bit Mainframe Node ID
    uint32_t stanag_seq_num;
    uint32_t crc32;
    size_t   payload_length;
    uint8_t  payload[STANAG_KERMIT_MAX_FRAME_LEN];
} tsfi_stanag_coaxial_frame_t;

typedef struct {
    uint32_t total_frames_sent;
    uint32_t total_kermit_acks_received;
    uint32_t crc_errors_detected;
    float    coaxial_wire_latency_ns;
    bool     carrier_detected;
} tsfi_stanag_coaxial_bus_state_t;

bool tsfi_stanag_coaxial_bus_init(tsfi_stanag_coaxial_bus_state_t *bus);
bool tsfi_stanag_coaxial_encapsulate(
    uint8_t sap,
    uint8_t priority,
    uint16_t node_id,
    const tsfi_kermit_packet_t *kermit_pkt,
    tsfi_stanag_coaxial_frame_t *frame_out
);
bool tsfi_stanag_coaxial_transmit(
    tsfi_stanag_coaxial_bus_state_t *bus,
    const tsfi_stanag_coaxial_frame_t *frame,
    uint8_t *wire_buffer,
    size_t *wire_len_out
);

#endif // TSFI_DEEPSEEK_STANAG_KERMIT_H
