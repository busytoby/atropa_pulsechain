#include <string.h>
#include "tsfi_parc_ethernet.h"

// Classical internet-style 16-bit packet checksum calculations
static uint16_t compute_pup_checksum(const uint8_t *stream, int len) {
    if (!stream || len <= 0) return 0;
    uint32_t sum = 0;
    for (int i = 0; i < len - 1; i += 2) {
        sum += (stream[i] << 8) | stream[i + 1];
    }
    if (len & 1) {
        sum += (stream[len - 1] << 8);
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (uint16_t)~sum;
}

int tsfi_parc_pup_encode(const tsfi_parc_pup_packet_t *pkt, uint8_t *stream_out, int max_len) {
    if (!pkt || !stream_out || pkt->data_len > 128 || max_len < 10 + pkt->data_len) return -1;

    stream_out[0] = pkt->dest_host;
    stream_out[1] = pkt->src_host;
    stream_out[2] = (pkt->pup_type >> 8) & 0xFF;
    stream_out[3] = pkt->pup_type & 0xFF;
    stream_out[4] = (pkt->pup_id >> 24) & 0xFF;
    stream_out[5] = (pkt->pup_id >> 16) & 0xFF;
    stream_out[6] = (pkt->pup_id >> 8) & 0xFF;
    stream_out[7] = pkt->pup_id & 0xFF;
    stream_out[8] = (pkt->data_len >> 8) & 0xFF;
    stream_out[9] = pkt->data_len & 0xFF;

    memcpy(stream_out + 10, pkt->payload, pkt->data_len);

    uint16_t chk = compute_pup_checksum(stream_out, 10 + pkt->data_len);
    stream_out[10 + pkt->data_len] = (chk >> 8) & 0xFF;
    stream_out[11 + pkt->data_len] = chk & 0xFF;

    return 12 + pkt->data_len;
}

int tsfi_parc_pup_decode(const uint8_t *stream_in, int stream_len, tsfi_parc_pup_packet_t *pkt_out) {
    if (!stream_in || !pkt_out || stream_len < 12) return -1;

    pkt_out->dest_host = stream_in[0];
    pkt_out->src_host = stream_in[1];
    pkt_out->pup_type = (stream_in[2] << 8) | stream_in[3];
    pkt_out->pup_id = (stream_in[4] << 24) | (stream_in[5] << 16) | (stream_in[6] << 8) | stream_in[7];
    pkt_out->data_len = (stream_in[8] << 8) | stream_in[9];
    if (pkt_out->data_len > 128) return -4; // payload bounds overflow protection

    if (stream_len < 12 + pkt_out->data_len) return -2; // stream too short for payload size

    memcpy(pkt_out->payload, stream_in + 10, pkt_out->data_len);

    uint16_t stream_chk = (stream_in[10 + pkt_out->data_len] << 8) | stream_in[11 + pkt_out->data_len];
    uint16_t computed = compute_pup_checksum(stream_in, 10 + pkt_out->data_len);

    pkt_out->checksum = stream_chk;
    if (stream_chk != computed) {
        return -3; // checksum error
    }

    return 0;
}
