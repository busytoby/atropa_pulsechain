#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define KERMIT_SOH 0x01
#define MAX_PAYLOAD 94

// Kermit Frame Structure
typedef struct {
    uint8_t soh;
    uint8_t len;
    uint8_t seq;
    uint8_t type;
    uint8_t data[MAX_PAYLOAD];
    uint8_t check;
} KermitFrame;

// Emulated SX1262 RF channel registry
typedef struct {
    uint8_t buffer[256];
    size_t size;
    bool carrier_active;
} VirtualRFChannel;

static VirtualRFChannel ch_a_to_b;
static VirtualRFChannel ch_b_to_a;

// Emulated Node States
typedef struct {
    char name[16];
    uint8_t seq_num;
    uint8_t received_data[MAX_PAYLOAD];
    size_t received_len;
} HeltecNode;

// Helper to compute Kermit 6-bit checksum
static uint8_t kermit_checksum(const uint8_t *buf, size_t len) {
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += buf[i];
    }
    return (uint8_t)(((sum + ((sum & 0xC0) >> 6)) & 0x3F) + 32);
}

// Pack a Kermit frame into binary OOK bytes
static size_t pack_kermit_frame(uint8_t seq, char type, const uint8_t *data, size_t data_len, uint8_t *out_buf) {
    out_buf[0] = KERMIT_SOH;
    out_buf[1] = (uint8_t)(data_len + 3 + 32); // length byte
    out_buf[2] = (uint8_t)(seq + 32);          // sequence number
    out_buf[3] = (uint8_t)type;                // packet type (D=Data, Y=ACK, N=NAK)
    
    if (data_len > 0 && data != NULL) {
        memcpy(&out_buf[4], data, data_len);
    }
    
    // Checksum includes everything except SOH and checksum itself
    uint8_t check = kermit_checksum(&out_buf[1], data_len + 3);
    out_buf[4 + data_len] = check;
    
    return data_len + 5;
}

// Parse/verify an incoming Kermit frame
static bool parse_kermit_frame(const uint8_t *buf, size_t size, KermitFrame *frame) {
    if (size < 5 || buf[0] != KERMIT_SOH) return false;
    
    frame->soh = buf[0];
    frame->len = (uint8_t)(buf[1] - 32);
    frame->seq = (uint8_t)(buf[2] - 32);
    frame->type = buf[3];
    
    size_t expected_data_len = frame->len - 3;
    if (expected_data_len > MAX_PAYLOAD || expected_data_len + 5 > size) return false;
    
    memcpy(frame->data, &buf[4], expected_data_len);
    frame->check = buf[4 + expected_data_len];
    
    // Verify checksum
    uint8_t calc_check = kermit_checksum(&buf[1], expected_data_len + 3);
    return calc_check == frame->check;
}

// Emulate physical OOK transmitter
static void sx1262_transmit_ook(VirtualRFChannel *chan, const uint8_t *data, size_t len) {
    // In OOK Mode, we physically modulate the carrier envelope
    chan->carrier_active = true;
    memcpy(chan->buffer, data, len);
    chan->size = len;
    printf("[SX1262] OOK Carrier Active: Transmitting %zu bytes envelope...\n", len);
}

// Emulate physical OOK receiver
static size_t sx1262_receive_ook(VirtualRFChannel *chan, uint8_t *out_data) {
    if (!chan->carrier_active || chan->size == 0) return 0;
    memcpy(out_data, chan->buffer, chan->size);
    size_t len = chan->size;
    
    // Reset channel state
    chan->carrier_active = false;
    chan->size = 0;
    return len;
}

int main() {
    printf("=== Auncient Heltec v4 ESP32-S3 OOK Kermit Interop Test ===\n");
    
    // Initialize emulated nodes
    HeltecNode node_a = { .name = "HELTEC_V4_A", .seq_num = 0, .received_len = 0 };
    HeltecNode node_b = { .name = "HELTEC_V4_B", .seq_num = 0, .received_len = 0 };
    
    // Prepare cryptographic handshake transaction payload (Node A Public Key)
    const uint8_t public_key_a[32] = {
        0xde, 0xad, 0xbe, 0xef, 0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c,
        0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
        0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c
    };
    
    uint8_t tx_buffer[256];
    uint8_t rx_buffer[256];
    
    // --- STEP 1: Node A sends handshake transaction data packet ---
    printf("\n[%s] Packaging cryptographic handshake packet...\n", node_a.name);
    size_t tx_len = pack_kermit_frame(node_a.seq_num, 'D', public_key_a, sizeof(public_key_a), tx_buffer);
    
    // Modulate over virtual OOK channel A-to-B
    sx1262_transmit_ook(&ch_a_to_b, tx_buffer, tx_len);
    
    // --- STEP 2: Node B scans carrier & decodes OOK packet ---
    printf("\n[%s] Listening for OOK envelope...\n", node_b.name);
    size_t rx_len = sx1262_receive_ook(&ch_a_to_b, rx_buffer);
    if (rx_len > 0) {
        KermitFrame parsed_frame;
        if (parse_kermit_frame(rx_buffer, rx_len, &parsed_frame)) {
            printf("[%s] [SUCCESS] Decoded valid Kermit packet. Seq: %d | Type: %c | Size: %d\n",
                   node_b.name, parsed_frame.seq, parsed_frame.type, parsed_frame.len - 3);
            
            // Capture received payload
            node_b.received_len = parsed_frame.len - 3;
            memcpy(node_b.received_data, parsed_frame.data, node_b.received_len);
            
            // --- STEP 3: Node B transmits ACK envelope (Kermit 'Y') ---
            printf("[%s] Transmitting ACK reply envelope...\n", node_b.name);
            size_t ack_len = pack_kermit_frame(parsed_frame.seq, 'Y', NULL, 0, tx_buffer);
            sx1262_transmit_ook(&ch_b_to_a, tx_buffer, ack_len);
        } else {
            printf("[%s] [ERROR] Failed to parse corrupted OOK packet.\n", node_b.name);
        }
    }
    
    // --- STEP 4: Node A receives ACK envelope ---
    printf("\n[%s] Scanning for reverse ACK envelope...\n", node_a.name);
    rx_len = sx1262_receive_ook(&ch_b_to_a, rx_buffer);
    if (rx_len > 0) {
        KermitFrame ack_frame;
        if (parse_kermit_frame(rx_buffer, rx_len, &ack_frame) && ack_frame.type == 'Y') {
            printf("[%s] [SUCCESS] Received ACK from receiver. Handshake sequence completed successfully!\n", node_a.name);
        } else {
            printf("[%s] [ERROR] ACK frame corrupted or missing.\n", node_a.name);
        }
    }
    
    printf("\n=== Interop Test Execution Finished ===\n");
    return 0;
}
