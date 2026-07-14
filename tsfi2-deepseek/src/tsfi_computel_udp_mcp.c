#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/*
 * Auncient Computel MCP-over-UDP Packet Simulator
 * 
 * Simulates connectionless UDP datagram delivery with packet drop handling,
 * sequence verification, and Kermit retransmission loops.
 */

#define UDP_MAX_PACKET 128

typedef struct {
    uint8_t seq;
    uint8_t type;
    uint8_t len;
    uint8_t payload[64];
    uint8_t checksum;
} McpUdpFrame;

typedef struct {
    uint8_t expected_seq;
    uint32_t packets_received;
    uint32_t packets_dropped;
    uint32_t retransmit_requests;
} UdpReceiverState;

void init_udp_receiver(UdpReceiverState *rcv) {
    if (!rcv) return;
    rcv->expected_seq = 0;
    rcv->packets_received = 0;
    rcv->packets_dropped = 0;
    rcv->retransmit_requests = 0;
}

/* Simple 6-bit checksum matching Kermit protocol standards */
uint8_t compute_mcp_checksum(const McpUdpFrame *frame) {
    uint32_t sum = frame->seq + frame->type + frame->len;
    for (int i = 0; i < frame->len; i++) {
        sum += frame->payload[i];
    }
    return (uint8_t)((sum & 0x3F) + 32);
}

/* Simulates transmitting a UDP datagram with a custom simulated drop flag */
bool simulate_udp_transmit(UdpReceiverState *rcv, const McpUdpFrame *frame, bool simulate_drop) {
    if (!rcv || !frame) return false;

    if (simulate_drop) {
        rcv->packets_dropped++;
        printf("[MCP UDP] Packet SEQ %d dropped due to channel noise.\n", frame->seq);
        // Fire NAK retransmit request back to sender
        rcv->retransmit_requests++;
        return false; // Packet lost
    }

    // Verify checksum
    uint8_t calc_check = compute_mcp_checksum(frame);
    if (frame->checksum != calc_check) {
        rcv->packets_dropped++;
        printf("[MCP UDP] Packet SEQ %d corrupted checksum (expected: %d, got: %d).\n", 
               frame->seq, frame->checksum, calc_check);
        rcv->retransmit_requests++;
        return false;
    }

    // Verify sequence
    if (frame->seq != rcv->expected_seq) {
        printf("[MCP UDP] Out-of-order packet SEQ %d (expected: %d). Requesting retransmission.\n", 
               frame->seq, rcv->expected_seq);
        rcv->retransmit_requests++;
        return false;
    }

    rcv->packets_received++;
    rcv->expected_seq++;
    printf("[MCP UDP] Packet SEQ %d successfully delivered (Payload: %s).\n", 
           frame->seq, frame->payload);
    
    return true;
}
