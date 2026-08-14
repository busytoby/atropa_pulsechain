#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tsfi_deepseek_stanag_kermit.h"

// ----------------------------------------------------------------------------
// 1. Coaxial Kermit Frame Encoding & Arithmetic Checksumming
// ----------------------------------------------------------------------------

static uint8_t compute_kermit_chk(const uint8_t *buf, size_t len) {
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += buf[i];
    }
    // Kermit 6-bit checksum algorithm: ((sum + ((sum & 0xC0) >> 6)) & 0x3F) + 32
    return (uint8_t)(((sum + ((sum & 0xC0) >> 6)) & 0x3F) + 32);
}

bool tsfi_kermit_packet_init(tsfi_kermit_packet_t *pkt, uint8_t seq, tsfi_kermit_packet_type_t type) {
    if (!pkt) return false;
    memset(pkt, 0, sizeof(*pkt));
    pkt->mark = STANAG_KERMIT_MARK_START;
    pkt->seq = (seq % 64) + 32;
    pkt->type = (uint8_t)type;
    return true;
}

bool tsfi_kermit_encode_frame(
    const tsfi_kermit_packet_t *pkt,
    uint8_t *raw_out,
    size_t *raw_out_len
) {
    if (!pkt || !raw_out || !raw_out_len) return false;
    
    // Total length = type(1) + data(N) + chk(1) + 32 bias
    uint8_t len_field = (uint8_t)(pkt->data_len + 2 + 32);
    
    raw_out[0] = pkt->mark;
    raw_out[1] = len_field;
    raw_out[2] = pkt->seq;
    raw_out[3] = pkt->type;
    if (pkt->data_len > 0) {
        memcpy(&raw_out[4], pkt->data, pkt->data_len);
    }
    
    // Checksum covers LEN, SEQ, TYPE, and DATA
    uint8_t chk = compute_kermit_chk(&raw_out[1], 3 + pkt->data_len);
    raw_out[4 + pkt->data_len] = chk;
    
    *raw_out_len = 5 + pkt->data_len;
    return true;
}

bool tsfi_kermit_decode_frame(
    const uint8_t *raw_in,
    size_t raw_len,
    tsfi_kermit_packet_t *pkt_out
) {
    if (!raw_in || raw_len < 5 || !pkt_out) return false;
    if (raw_in[0] != STANAG_KERMIT_MARK_START) return false;

    uint8_t len_field = raw_in[1];
    if (len_field < 34) return false;
    size_t dlen = (size_t)(len_field - 34);
    if (raw_len < 5 + dlen) return false;

    uint8_t expected_chk = compute_kermit_chk(&raw_in[1], 3 + dlen);
    if (raw_in[4 + dlen] != expected_chk) {
        return false; // Checksum mismatch
    }

    pkt_out->mark = raw_in[0];
    pkt_out->length = raw_in[1];
    pkt_out->seq = raw_in[2];
    pkt_out->type = raw_in[3];
    pkt_out->data_len = dlen;
    if (dlen > 0) {
        memcpy(pkt_out->data, &raw_in[4], dlen);
    }
    pkt_out->checksum = raw_in[4 + dlen];
    return true;
}

// ----------------------------------------------------------------------------
// 2. STANAG 5066 / 5516 Coaxial Bus Encapsulator & CRC32
// ----------------------------------------------------------------------------

static uint32_t stanag_crc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int k = 0; k < 8; k++) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
    }
    return ~crc;
}

bool tsfi_stanag_coaxial_bus_init(tsfi_stanag_coaxial_bus_state_t *bus) {
    if (!bus) return false;
    memset(bus, 0, sizeof(*bus));
    bus->carrier_detected = true;
    return true;
}

bool tsfi_stanag_coaxial_encapsulate(
    uint8_t sap,
    uint8_t priority,
    uint16_t node_id,
    const tsfi_kermit_packet_t *kermit_pkt,
    tsfi_stanag_coaxial_frame_t *frame_out
) {
    if (!kermit_pkt || !frame_out) return false;
    memset(frame_out, 0, sizeof(*frame_out));

    uint8_t encoded_kermit[STANAG_KERMIT_MAX_FRAME_LEN];
    size_t kermit_len = 0;
    if (!tsfi_kermit_encode_frame(kermit_pkt, encoded_kermit, &kermit_len)) {
        return false;
    }

    frame_out->sap_address = sap;
    frame_out->priority = priority & 0x0F;
    frame_out->node_address = node_id;
    frame_out->payload_length = kermit_len;
    memcpy(frame_out->payload, encoded_kermit, kermit_len);
    frame_out->crc32 = stanag_crc32(encoded_kermit, kermit_len);
    return true;
}

bool tsfi_stanag_coaxial_transmit(
    tsfi_stanag_coaxial_bus_state_t *bus,
    const tsfi_stanag_coaxial_frame_t *frame,
    uint8_t *wire_buffer,
    size_t *wire_len_out
) {
    if (!bus || !frame || !wire_buffer || !wire_len_out) return false;
    struct timespec ts0, ts1;
    clock_gettime(CLOCK_MONOTONIC, &ts0);

    // STANAG 5066 Physical Header: [SYNC 0x57A1] [SAP 1B] [PRIO 1B] [NODE 2B] [LEN 2B] [PAYLOAD] [CRC32 4B]
    wire_buffer[0] = 0x57;
    wire_buffer[1] = 0xA1;
    wire_buffer[2] = frame->sap_address;
    wire_buffer[3] = frame->priority;
    wire_buffer[4] = (uint8_t)(frame->node_address & 0xFF);
    wire_buffer[5] = (uint8_t)((frame->node_address >> 8) & 0xFF);
    wire_buffer[6] = (uint8_t)(frame->payload_length & 0xFF);
    wire_buffer[7] = (uint8_t)((frame->payload_length >> 8) & 0xFF);

    memcpy(&wire_buffer[8], frame->payload, frame->payload_length);
    size_t off = 8 + frame->payload_length;

    memcpy(&wire_buffer[off], &frame->crc32, sizeof(uint32_t));
    *wire_len_out = off + sizeof(uint32_t);

    bus->total_frames_sent++;
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    bus->coaxial_wire_latency_ns = (float)((ts1.tv_sec - ts0.tv_sec) * 1e9 + (ts1.tv_nsec - ts0.tv_nsec));
    return true;
}
