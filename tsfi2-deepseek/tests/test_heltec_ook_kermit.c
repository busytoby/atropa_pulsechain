#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define KERMIT_SOH 0x01
#define MAX_PAYLOAD 90

// Kermit Frame Structure with 2-byte addressing fields
typedef struct {
    uint8_t soh;
    uint8_t len;
    uint8_t seq;
    uint8_t type;
    uint16_t src_addr;
    uint16_t dst_addr;
    uint8_t data[MAX_PAYLOAD];
    uint8_t check;
} KermitFrame;

// Emulated Shared Broadcast RF Channel
typedef struct {
    uint8_t buffer[256];
    size_t size;
    bool carrier_active;
} BroadcastRFChannel;

static BroadcastRFChannel shared_channel;

// Emulated Node States
typedef struct {
    char name[16];
    uint16_t address;
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

// Pack an addressed Kermit frame into binary OOK bytes
static size_t pack_addressed_frame(uint8_t seq, char type, uint16_t src, uint16_t dst, const uint8_t *data, size_t data_len, uint8_t *out_buf) {
    out_buf[0] = KERMIT_SOH;
    out_buf[1] = (uint8_t)(data_len + 7 + 32); // length byte (data_len + 4 bytes addresses + 3 header bytes)
    out_buf[2] = (uint8_t)(seq + 32);          // sequence
    out_buf[3] = (uint8_t)type;                // type
    
    // Add Addressing fields
    out_buf[4] = (uint8_t)(src & 0xFF);
    out_buf[5] = (uint8_t)((src >> 8) & 0xFF);
    out_buf[6] = (uint8_t)(dst & 0xFF);
    out_buf[7] = (uint8_t)((dst >> 8) & 0xFF);
    
    if (data_len > 0 && data != NULL) {
        memcpy(&out_buf[8], data, data_len);
    }
    
    uint8_t check = kermit_checksum(&out_buf[1], data_len + 7);
    out_buf[8 + data_len] = check;
    
    return data_len + 9;
}

// Parse/verify an incoming addressed Kermit frame
static bool parse_addressed_frame(const uint8_t *buf, size_t size, KermitFrame *frame) {
    if (size < 9 || buf[0] != KERMIT_SOH) return false;
    
    frame->soh = buf[0];
    frame->len = (uint8_t)(buf[1] - 32);
    frame->seq = (uint8_t)(buf[2] - 32);
    frame->type = buf[3];
    
    frame->src_addr = buf[4] | (buf[5] << 8);
    frame->dst_addr = buf[6] | (buf[7] << 8);
    
    size_t expected_data_len = frame->len - 7;
    if (expected_data_len > MAX_PAYLOAD || expected_data_len + 9 > size) return false;
    
    memcpy(frame->data, &buf[8], expected_data_len);
    frame->check = buf[8 + expected_data_len];
    
    uint8_t calc_check = kermit_checksum(&buf[1], expected_data_len + 7);
    return calc_check == frame->check;
}

// Broadcast packet onto the shared channel
static void transmit_broadcast(const uint8_t *data, size_t len) {
    shared_channel.carrier_active = true;
    memcpy(shared_channel.buffer, data, len);
    shared_channel.size = len;
    printf("[BROADCAST] Transmitting %zu bytes envelope over shared RF spectrum...\n", len);
}

// Listen / scan the shared broadcast channel
static size_t receive_broadcast(uint8_t *out_data) {
    if (!shared_channel.carrier_active || shared_channel.size == 0) return 0;
    memcpy(out_data, shared_channel.buffer, shared_channel.size);
    return shared_channel.size;
}

static void clear_channel() {
    shared_channel.carrier_active = false;
    shared_channel.size = 0;
}

int main() {
    printf("=== Auncient Multi-Party OOK Kermit Address Filtering Test ===\n");
    
    // Initialize 3 nodes in range of each other
    HeltecNode node_a = { .name = "NODE_A", .address = 0xAA01, .seq_num = 0 };
    HeltecNode node_b = { .name = "NODE_B", .address = 0xBB02, .seq_num = 0 };
    HeltecNode node_c = { .name = "NODE_C", .address = 0xCC03, .seq_num = 0 };
    
    uint8_t tx_buffer[256];
    uint8_t rx_buffer[256];
    
    const uint8_t secret_payload[32] = "Auncient-Secure-Multicast-Data";
    
    // --- STEP 1: Node A broadcasts a packet addressed explicitly to Node B ---
    printf("\n[%s] Transmitting payload addressed to %s (0x%04X)...\n", node_a.name, node_b.name, node_b.address);
    size_t tx_len = pack_addressed_frame(node_a.seq_num, 'D', node_a.address, node_b.address, secret_payload, sizeof(secret_payload), tx_buffer);
    transmit_broadcast(tx_buffer, tx_len);
    
    // --- STEP 2: Node C (unintended recipient) listens to the channel ---
    size_t rx_len = receive_broadcast(rx_buffer);
    if (rx_len > 0) {
        KermitFrame parsed_frame;
        if (parse_addressed_frame(rx_buffer, rx_len, &parsed_frame)) {
            printf("[%s] Heard packet on channel. Destination Address: 0x%04X\n", node_c.name, parsed_frame.dst_addr);
            if (parsed_frame.dst_addr == node_c.address) {
                printf("[%s] [ACCEPT] Packet is addressed to me! Processing...\n", node_c.name);
            } else {
                printf("[%s] [DISCARD] Destination mismatch! Ignoring packet to protect privacy.\n", node_c.name);
            }
        }
    }
    
    // --- STEP 3: Node B (intended recipient) listens to the channel ---
    rx_len = receive_broadcast(rx_buffer);
    if (rx_len > 0) {
        KermitFrame parsed_frame;
        if (parse_addressed_frame(rx_buffer, rx_len, &parsed_frame)) {
            printf("[%s] Heard packet on channel. Destination Address: 0x%04X\n", node_b.name, parsed_frame.dst_addr);
            if (parsed_frame.dst_addr == node_b.address) {
                printf("[%s] [ACCEPT] Packet matches my address! Saving payload...\n", node_b.name);
                memcpy(node_b.received_data, parsed_frame.data, sizeof(secret_payload));
                node_b.received_len = sizeof(secret_payload);
                
                // Clear active channel to reply
                clear_channel();
                
                // Reply with ACK addressed back to Node A
                printf("[%s] Transmitting ACK addressed back to %s (0x%04X)...\n", node_b.name, node_a.name, node_a.address);
                size_t ack_len = pack_addressed_frame(parsed_frame.seq, 'Y', node_b.address, node_a.address, NULL, 0, tx_buffer);
                transmit_broadcast(tx_buffer, ack_len);
            }
        }
    }
    
    // --- STEP 4: Node A listens for incoming ACK verification ---
    rx_len = receive_broadcast(rx_buffer);
    if (rx_len > 0) {
        KermitFrame ack_frame;
        if (parse_addressed_frame(rx_buffer, rx_len, &ack_frame)) {
            printf("\n[%s] Received ACK frame. Source: 0x%04X | Destination: 0x%04X\n", node_a.name, ack_frame.src_addr, ack_frame.dst_addr);
            if (ack_frame.dst_addr == node_a.address && ack_frame.src_addr == node_b.address) {
                printf("[%s] [SUCCESS] ACK verified from target peer %s (0x%04X). Handshake complete!\n", 
                       node_a.name, node_b.name, node_b.address);
            } else {
                printf("[%s] [IGNORE] Spoofed or misrouted ACK detected. Dropping packet.\n", node_a.name);
            }
        }
    }
    
    clear_channel();
    printf("\n=== Multi-Party Address Verification Completed Successfully ===\n");
    return 0;
}
