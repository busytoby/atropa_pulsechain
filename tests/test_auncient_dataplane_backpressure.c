#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <assert.h>

#define PAYLOAD_SIZE 64
#define WINDOW_SIZE 4
#define QUEUE_MAX 8

typedef struct {
    uint8_t packet_type;        // 0 = DATA, 1 = ACK, 2 = BACKPRESSURE_ON, 3 = BACKPRESSURE_OFF
    uint64_t sequence_counter;
    uint8_t payload[PAYLOAD_SIZE];
} flow_packet_t;

typedef struct {
    uint64_t send_base;
    uint64_t next_seq;
    bool backpressure_active;
} tx_window_t;

typedef struct {
    uint64_t expected_seq;
    int queue_count;
    int high_water_mark;
    int low_water_mark;
    bool backpressure_sent;
} rx_queue_t;

static void init_tx_window(tx_window_t *tx) {
    tx->send_base = 1;
    tx->next_seq = 1;
    tx->backpressure_active = false;
}

static void init_rx_queue(rx_queue_t *rx) {
    rx->expected_seq = 1;
    rx->queue_count = 0;
    rx->high_water_mark = 3;  // Trigger backpressure at 3 packets in queue
    rx->low_water_mark = 1;   // Clear backpressure at 1 packet in queue
    rx->backpressure_sent = false;
}

// Check if sliding window is open
static bool is_window_open(const tx_window_t *tx) {
    if (tx->backpressure_active) {
        return false; // Backpressure blocks transmission
    }
    return (tx->next_seq < tx->send_base + WINDOW_SIZE);
}

// Send data packet if sliding window is open
static bool tx_send_packet(tx_window_t *tx, int socket_fd, const uint8_t *data) {
    if (!is_window_open(tx)) {
        return false; // Blocked
    }

    flow_packet_t packet;
    packet.packet_type = 0; // DATA
    packet.sequence_counter = tx->next_seq++;
    memcpy(packet.payload, data, PAYLOAD_SIZE);

    if (write(socket_fd, &packet, sizeof(flow_packet_t)) < 0) {
        return false;
    }
    return true;
}

// Process incoming feedback control packets in a loop
static void tx_process_feedback(tx_window_t *tx, int socket_fd) {
    flow_packet_t packet;
    // Read all non-blocking simulated feedback packets in buffer
    while (recv(socket_fd, &packet, sizeof(flow_packet_t), MSG_DONTWAIT) >= 0) {
        if (packet.packet_type == 1) { // ACK
            if (packet.sequence_counter >= tx->send_base) {
                tx->send_base = packet.sequence_counter + 1;
            }
        } else if (packet.packet_type == 2) { // BACKPRESSURE_ON
            tx->backpressure_active = true;
        } else if (packet.packet_type == 3) { // BACKPRESSURE_OFF
            tx->backpressure_active = false;
        }
    }
}

// Ingest packet at Receiver and monitor queue watermarks
static bool rx_ingest_packet(rx_queue_t *rx, int socket_in_fd, int socket_out_fd) {
    flow_packet_t packet;
    if (read(socket_in_fd, &packet, sizeof(flow_packet_t)) < 0) {
        return false;
    }

    if (packet.packet_type != 0) {
        return false; // Expecting DATA packet only
    }

    if (packet.sequence_counter != rx->expected_seq) {
        return false; // Out-of-order drop
    }

    rx->expected_seq++;
    rx->queue_count++;

    // Check high-water mark for backpressure trigger
    if (rx->queue_count >= rx->high_water_mark && !rx->backpressure_sent) {
        rx->backpressure_sent = true;
        flow_packet_t bp;
        bp.packet_type = 2; // BACKPRESSURE_ON
        bp.sequence_counter = 0;
        write(socket_out_fd, &bp, sizeof(flow_packet_t));
    }

    return true;
}

// Release/consume packet from Receiver queue and send ACK
static bool rx_release_packet(rx_queue_t *rx, int socket_out_fd, uint64_t *acked_seq) {
    if (rx->queue_count == 0) {
        return false;
    }

    // Process/consume oldest packet
    rx->queue_count--;
    *acked_seq = rx->expected_seq - rx->queue_count - 1;

    // Send ACK packet back
    flow_packet_t ack;
    ack.packet_type = 1; // ACK
    ack.sequence_counter = *acked_seq;
    write(socket_out_fd, &ack, sizeof(flow_packet_t));

    // Check low-water mark to clear backpressure
    if (rx->queue_count <= rx->low_water_mark && rx->backpressure_sent) {
        rx->backpressure_sent = false;
        flow_packet_t bp;
        bp.packet_type = 3; // BACKPRESSURE_OFF
        bp.sequence_counter = 0;
        write(socket_out_fd, &bp, sizeof(flow_packet_t));
    }

    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT DATAPLANE BACKPRESSURE & WINDOW FLOW CONTROL SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    int fds[2];
    bool ok = (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) >= 0);
    assert(ok == true);

    tx_window_t tx;
    init_tx_window(&tx);

    rx_queue_t rx;
    init_rx_queue(&rx);

    uint8_t payload[PAYLOAD_SIZE];
    memset(payload, 0x3C, PAYLOAD_SIZE);

    // 1. Fill sliding window to capacity limits (Window size is 4)
    printf("[TEST] Filling sliding window to capacity limits...\n");
    fflush(stdout);
    
    // First 4 packets must succeed
    assert(tx_send_packet(&tx, fds[0], payload) == true); // Seq 1
    assert(tx_send_packet(&tx, fds[0], payload) == true); // Seq 2
    assert(tx_send_packet(&tx, fds[0], payload) == true); // Seq 3
    assert(tx_send_packet(&tx, fds[0], payload) == true); // Seq 4
    
    // 5th packet must block due to window saturation
    assert(tx_send_packet(&tx, fds[0], payload) == false);
    printf("   ✓ Sliding window exhausted. 5th packet blocked successfully.\n");
    fflush(stdout);

    // 2. Ingest packets on receiver and send ACKs to advance window
    printf("[TEST] Consuming packets and sending ACKs to advance window...\n");
    fflush(stdout);

    // Ingest seq 1 and 2
    assert(rx_ingest_packet(&rx, fds[1], fds[1]) == true);
    assert(rx_ingest_packet(&rx, fds[1], fds[1]) == true);
    
    // Release and ACK seq 1
    uint64_t ack_seq = 0;
    assert(rx_release_packet(&rx, fds[1], &ack_seq) == true);
    assert(ack_seq == 1);

    // Transmitter processes feedback
    tx_process_feedback(&tx, fds[0]);
    assert(tx.send_base == 2); // Base advanced to 2
    
    // Window should now be open for seq 5
    assert(tx_send_packet(&tx, fds[0], payload) == true); // Seq 5
    assert(tx_send_packet(&tx, fds[0], payload) == false); // Blocks on seq 6
    printf("   ✓ ACK advanced sliding window. Transmitted sequence 5 successfully.\n");
    fflush(stdout);

    // 3. Trigger High-Water Backpressure
    printf("[TEST] Testing high-water backpressure trigger...\n");
    fflush(stdout);

    // Current queue size is 1 (seq 2 is unconsumed, seq 1 was consumed, seq 3, 4, 5 are in flight)
    // Ingest seq 3, 4, 5. Receiver queue size will grow to 4 (seq 2, 3, 4, 5)
    assert(rx_ingest_packet(&rx, fds[1], fds[1]) == true); // Queue = 2
    assert(rx_ingest_packet(&rx, fds[1], fds[1]) == true); // Queue = 3 (Hits high-water mark 3, sends backpressure)
    assert(rx_ingest_packet(&rx, fds[1], fds[1]) == true); // Queue = 4

    assert(rx.backpressure_sent == true);

    // Process feedback at transmitter to activate backpressure
    tx_process_feedback(&tx, fds[0]); // Receives ACK of seq 1, ACKs and backpressure ON
    assert(tx.backpressure_active == true);

    // Even if we process further ACKs to advance window base, backpressure must block transmission!
    assert(rx_release_packet(&rx, fds[1], &ack_seq) == true); // ACK seq 2, Queue drops to 3
    tx_process_feedback(&tx, fds[0]); // Process ACK seq 2
    
    assert(tx_send_packet(&tx, fds[0], payload) == false); // Still blocked by backpressure!
    printf("   ✓ Backpressure active. Transmission blocked despite open window.\n");
    fflush(stdout);

    // 4. Drain queue to low-water mark to clear backpressure
    printf("[TEST] Draining receiver queue to clear backpressure...\n");
    fflush(stdout);

    assert(rx_release_packet(&rx, fds[1], &ack_seq) == true); // ACK seq 3, Queue drops to 2
    assert(rx_release_packet(&rx, fds[1], &ack_seq) == true); // ACK seq 4, Queue drops to 1 (Hits low-water, clears backpressure)
    assert(rx.backpressure_sent == false);

    // Process feedback at transmitter
    tx_process_feedback(&tx, fds[0]); // Process remaining ACKs and BACKPRESSURE_OFF
    assert(tx.backpressure_active == false);

    // Transmission should now resume
    assert(tx_send_packet(&tx, fds[0], payload) == true); // Seq 6
    printf("   ✓ Receiver queue drained. Backpressure cleared and transmission resumed.\n");
    fflush(stdout);

    close(fds[0]);
    close(fds[1]);

    printf("=============================================================\n");
    printf("BACKPRESSURE & FLOW CONTROL TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
