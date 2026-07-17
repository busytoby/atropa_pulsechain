#include "tsfi_mainframe_decnet.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>


int tsfi_sdlc_deserialize(const uint8_t *buf, size_t len, tsfi_sdlc_frame *frame_out) {
    if (!buf || !frame_out || len < 6) return -1;
    if (buf[0] != 0x7E || buf[len - 1] != 0x7E) return -2;
    
    frame_out->address = buf[1];
    frame_out->control = buf[2];
    frame_out->payload_len = len - 6;
    if (frame_out->payload_len > 256) return -3;
    
    memcpy(frame_out->payload, buf + 3, frame_out->payload_len);
    
    uint16_t crc_rx = (buf[len - 3] << 8) | buf[len - 2];
    uint16_t crc_calc = tsfi_sdlc_crc16(buf + 1, 2 + frame_out->payload_len);
    frame_out->fcs = crc_rx;
    
    if (crc_rx != crc_calc) return -4;
    return 0;
}

int tsfi_decnet_deserialize_header(const uint8_t *buf, size_t len, tsfi_decnet_header *hdr_out) {
    if (!buf || !hdr_out || len < 6) return -1;
    hdr_out->flags = buf[0];
    hdr_out->dst_node = (buf[1] << 8) | buf[2];
    hdr_out->src_node = (buf[3] << 8) | buf[4];
    hdr_out->forward_count = buf[5];
    return 0;
}

int tsfi_nsp_deserialize_control(const uint8_t *buf, size_t len, tsfi_nsp_control_msg *msg_out) {
    if (!buf || !msg_out || len < 6) return -1;
    msg_out->command = buf[0];
    msg_out->src_link = (buf[1] << 8) | buf[2];
    msg_out->dst_link = (buf[3] << 8) | buf[4];
    msg_out->services = buf[5];
    return 0;
}

int tsfi_s240_audit(const char *user, int action_type, const char *resource, char *log_out, size_t max_len) {
    if (!user || !resource || !log_out || max_len == 0) return -1;
    const char *action_str = "UNKNOWN";
    if (action_type == S240_ACCESS) action_str = "UNAUTHORIZED_ACCESS";
    else if (action_type == S240_MODIFY) action_str = "UNAUTHORIZED_MODIFICATION";
    else if (action_type == S240_DESTROY) action_str = "UNAUTHORIZED_DESTRUCTION";
    
    snprintf(log_out, max_len, "S240_AUDIT: User=%s, Action=%s, Resource=%s", user, action_str, resource);
    return 0;
}

int tsfi_apollo_deserialize(const uint8_t *buf, size_t len, tsfi_apollo_frame *frame_out) {
    if (!buf || !frame_out || len < 5) return -1;
    frame_out->ring_id = (buf[0] << 8) | buf[1];
    frame_out->source_node = buf[2];
    frame_out->dest_node = buf[3];
    frame_out->control_token = buf[4];
    return 0;
}

uint8_t tsfi_secded_encode(uint8_t data) {
    uint8_t d0 = (data >> 0) & 1;
    uint8_t d1 = (data >> 1) & 1;
    uint8_t d2 = (data >> 2) & 1;
    uint8_t d3 = (data >> 3) & 1;
    uint8_t d4 = (data >> 4) & 1;
    uint8_t d5 = (data >> 5) & 1;
    uint8_t d6 = (data >> 6) & 1;
    uint8_t d7 = (data >> 7) & 1;

    uint8_t p1 = d0 ^ d1 ^ d3 ^ d4 ^ d6;
    uint8_t p2 = d0 ^ d2 ^ d3 ^ d5 ^ d6;
    uint8_t p3 = d1 ^ d2 ^ d3 ^ d7;
    uint8_t p4 = d4 ^ d5 ^ d6 ^ d7;

    return (p1 << 0) | (p2 << 1) | (p3 << 2) | (p4 << 3);
}

int tsfi_secded_decode(uint8_t data, uint8_t parity, uint8_t *corrected_data_out) {
    if (!corrected_data_out) return -1;
    uint8_t expected = tsfi_secded_encode(data);
    uint8_t syndrome = expected ^ parity;
    if (syndrome == 0) {
        *corrected_data_out = data;
        return 0;
    }
    if (syndrome == 0x03) {
        *corrected_data_out = data ^ (1 << 0);
        return 1;
    }
    *corrected_data_out = data;
    return -2;
}

int tsfi_cyclades_serialize(const tsfi_cyclades_header *hdr, uint8_t *buf, size_t *len_out) {
    if (!hdr || !buf || !len_out) return -1;
    buf[0] = hdr->src_node;
    buf[1] = hdr->dest_node;
    buf[2] = (hdr->seq_num >> 8) & 0xFF;
    buf[3] = hdr->seq_num & 0xFF;
    buf[4] = hdr->flags;
    *len_out = 5;
    return 0;
}

int tsfi_cyclades_deserialize(const uint8_t *buf, size_t len, tsfi_cyclades_header *hdr_out) {
    if (!buf || !hdr_out || len < 5) return -1;
    hdr_out->src_node = buf[0];
    hdr_out->dest_node = buf[1];
    hdr_out->seq_num = (buf[2] << 8) | buf[3];
    hdr_out->flags = buf[4];
    return 0;
}

void tsfi_cyclades_conn_init(tsfi_cyclades_connection *conn) {
    if (!conn) return;
    conn->state = CYCLADES_STATE_CLOSED;
    conn->local_seq = 100;
    conn->remote_seq = 0;
}

int tsfi_cyclades_process_packet(tsfi_cyclades_connection *conn, const tsfi_cyclades_header *packet_in, tsfi_cyclades_header *packet_out) {
    if (!conn || !packet_in || !packet_out) return -1;
    if (conn->state == CYCLADES_STATE_CLOSED && (packet_in->flags == 0x02)) {
        conn->state = CYCLADES_STATE_SYN_RCVD;
        conn->remote_seq = packet_in->seq_num;
        packet_out->flags = 0x12;
        packet_out->seq_num = conn->local_seq;
        packet_out->src_node = packet_in->dest_node;
        packet_out->dest_node = packet_in->src_node;
        return 0;
    } else if (conn->state == CYCLADES_STATE_SYN_RCVD && (packet_in->flags == 0x10)) {
        conn->state = CYCLADES_STATE_ESTABLISHED;
        conn->local_seq++;
        return 0;
    }
    return -2;
}

int tsfi_cyclades_window_verify(const tsfi_cyclades_window *win, uint16_t seq) {
    if (!win) return -1;
    uint16_t offset = seq - win->window_start;
    return (offset < win->window_size) ? 0 : -2;
}

