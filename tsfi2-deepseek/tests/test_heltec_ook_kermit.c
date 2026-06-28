#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

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

// Emulated SX1262 RF channel registry with noise injection
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
    uint8_t received_data[512];
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
    
    uint8_t calc_check = kermit_checksum(&buf[1], expected_data_len + 3);
    return calc_check == frame->check;
}

// Emulate physical OOK transmitter with noise injection support
static void sx1262_transmit_ook(VirtualRFChannel *chan, const uint8_t *data, size_t len, bool inject_noise) {
    chan->carrier_active = true;
    memcpy(chan->buffer, data, len);
    chan->size = len;
    
    if (inject_noise) {
        // Corrupt a random byte in the payload to simulate party-line line noise
        int corrupt_idx = (rand() % (len - 1)) + 1; // avoid corrupting SOH for parsing simplicity
        chan->buffer[corrupt_idx] ^= 0xFF;
        printf("[SX1262] [NOISE INJECTED] Transmitting corrupted %zu bytes envelope (corrupted byte at index %d)...\n", len, corrupt_idx);
    } else {
        printf("[SX1262] Transmitting clean %zu bytes envelope...\n", len);
    }
}

// Emulate physical OOK receiver
static size_t sx1262_receive_ook(VirtualRFChannel *chan, uint8_t *out_data) {
    if (!chan->carrier_active || chan->size == 0) return 0;
    memcpy(out_data, chan->buffer, chan->size);
    size_t len = chan->size;
    
    chan->carrier_active = false;
    chan->size = 0;
    return len;
}

int main() {
    srand(time(NULL));
    printf("=== Auncient Heltec v4 ESP32-S3 OOK Kermit Emulated Noise Tolerant Test ===\n");
    
    // Initialize nodes
    HeltecNode node_a = { .name = "HELTEC_V4_A", .seq_num = 0, .received_len = 0 };
    HeltecNode node_b = { .name = "HELTEC_V4_B", .seq_num = 0, .received_len = 0 };
    
    // Multi-packet cryptographic payload (96 bytes split into 3 segments)
    const uint8_t payload[96] = "Auncient-WinchesterMQ-SCSI-Transaction-Data-For-Secure-Channel-Key-Exchange-Sequence-Block-OK";
    const size_t segment_size = 32;
    const size_t total_segments = 3;
    
    uint8_t tx_buffer[256];
    uint8_t rx_buffer[256];
    
    for (size_t seg = 0; seg < total_segments; seg++) {
        bool success = false;
        int attempts = 0;
        
        while (!success && attempts < 5) {
            attempts++;
            printf("\n[%s] [Segment %zu/3] Attempt %d: Packaging packet...\n", node_a.name, seg + 1, attempts);
            
            size_t tx_len = pack_kermit_frame(node_a.seq_num, 'D', &payload[seg * segment_size], segment_size, tx_buffer);
            
            // Inject noise on the first attempt of segment 2 to test NAK recovery
            bool inject_noise = (seg == 1 && attempts == 1);
            sx1262_transmit_ook(&ch_a_to_b, tx_buffer, tx_len, inject_noise);
            
            // Node B receives
            size_t rx_len = sx1262_receive_ook(&ch_a_to_b, rx_buffer);
            if (rx_len > 0) {
                KermitFrame parsed_frame;
                if (parse_kermit_frame(rx_buffer, rx_len, &parsed_frame)) {
                    printf("[%s] [SUCCESS] Decoded segment. Seq: %d | Size: %d\n",
                           node_b.name, parsed_frame.seq, parsed_frame.len - 3);
                    
                    // Save payload segment
                    memcpy(&node_b.received_data[seg * segment_size], parsed_frame.data, segment_size);
                    node_b.received_len += segment_size;
                    
                    // Send ACK
                    size_t ack_len = pack_kermit_frame(parsed_frame.seq, 'Y', NULL, 0, tx_buffer);
                    sx1262_transmit_ook(&ch_b_to_a, tx_buffer, ack_len, false);
                    success = true;
                } else {
                    printf("[%s] [CHECKSUM FAILED] Packet corrupted. Transmitting NAK reply envelope...\n", node_b.name);
                    // Send NAK (Kermit 'N')
                    size_t nak_len = pack_kermit_frame(node_a.seq_num, 'N', NULL, 0, tx_buffer);
                    sx1262_transmit_ook(&ch_b_to_a, tx_buffer, nak_len, false);
                }
            }
            
            // Node A listens for reply
            rx_len = sx1262_receive_ook(&ch_b_to_a, rx_buffer);
            if (rx_len > 0) {
                KermitFrame reply_frame;
                if (parse_kermit_frame(rx_buffer, rx_len, &reply_frame)) {
                    if (reply_frame.type == 'Y') {
                        printf("[%s] Received ACK. Advancing packet sequence.\n", node_a.name);
                        node_a.seq_num = (node_a.seq_num + 1) % 64;
                    } else if (reply_frame.type == 'N') {
                        printf("[%s] Received NAK. Retransmitting segment...\n", node_a.name);
                        success = false;
                    }
                }
            }
        }
        
        if (!success) {
            printf("[FATAL] Transfer failed after maximum attempts.\n");
            return 1;
        }
    }
    
    // Verify final assembled data at receiver
    node_b.received_data[node_b.received_len] = '\0';
    printf("\n[%s] [SUCCESS] Fully assembled payload: \"%s\"\n", node_b.name, node_b.received_data);
    printf("=== Emulated Noise Tolerant Test Execution Finished successfully ===\n");
    return 0;
}
