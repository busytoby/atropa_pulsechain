#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <assert.h>

#define STANAG_PROTO 1
#define DECNET_PROTO 2
#define PAYLOAD_SIZE 64

typedef struct {
    uint8_t protocol_type;      // STANAG_PROTO or DECNET_PROTO
    uint64_t sequence_counter;  // Monotonic sequence number
    uint8_t payload[PAYLOAD_SIZE];
    uint16_t checksum;          // Simple validation checksum
} dataplane_packet_t;

typedef struct {
    int socket_fds[2];
    uint64_t last_stanag_seq;
    uint64_t last_decnet_seq;
} dataplane_multiplexer_t;

// Simple checksum generator (XOR based)
static uint16_t calculate_checksum(const uint8_t *data, size_t len) {
    uint16_t checksum = 0;
    for (size_t i = 0; i < len; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

// Initialize coaxial loopback links for the Dataplane
static bool init_dataplane_multiplexer(dataplane_multiplexer_t *mux) {
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, mux->socket_fds) < 0) {
        return false;
    }
    mux->last_stanag_seq = 0;
    mux->last_decnet_seq = 0;
    return true;
}

// Ingest and transmit packet over Dataplane loopback coaxial links
static bool transmit_dataplane_packet(dataplane_multiplexer_t *mux, uint8_t proto, uint64_t seq, const uint8_t *data) {
    dataplane_packet_t packet;
    packet.protocol_type = proto;
    packet.sequence_counter = seq;
    memcpy(packet.payload, data, PAYLOAD_SIZE);
    packet.checksum = calculate_checksum(data, PAYLOAD_SIZE);

    // Write packet over coaxial output
    if (write(mux->socket_fds[0], &packet, sizeof(dataplane_packet_t)) < 0) {
        return false;
    }
    return true;
}

// Receive and process packets on the Dataplane (Independent of Control Plane Locks)
static bool receive_and_route_dataplane(dataplane_multiplexer_t *mux, dataplane_packet_t *rx_packet) {
    if (read(mux->socket_fds[1], rx_packet, sizeof(dataplane_packet_t)) < 0) {
        return false;
    }

    // 1. Checksum validation
    uint16_t computed = calculate_checksum(rx_packet->payload, PAYLOAD_SIZE);
    if (computed != rx_packet->checksum) {
        return false; // Trapped corrupted packet
    }

    // 2. Sequence monotonicity verification by protocol
    if (rx_packet->protocol_type == STANAG_PROTO) {
        if (rx_packet->sequence_counter <= mux->last_stanag_seq) {
            return false; // Out-of-order sequence regression rejected
        }
        mux->last_stanag_seq = rx_packet->sequence_counter;
    } else if (rx_packet->protocol_type == DECNET_PROTO) {
        if (rx_packet->sequence_counter <= mux->last_decnet_seq) {
            return false;
        }
        mux->last_decnet_seq = rx_packet->sequence_counter;
    } else {
        return false; // Unknown protocol
    }

    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT DATAPLANE MULTIPLEXER VERIFICATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    dataplane_multiplexer_t mux;
    bool ok = init_dataplane_multiplexer(&mux);
    assert(ok == true);
    printf("   ✓ Dataplane coaxial loopback link initialized.\n");
    fflush(stdout);

    uint8_t sample_payload[PAYLOAD_SIZE];
    for (int i = 0; i < PAYLOAD_SIZE; i++) {
        sample_payload[i] = (uint8_t)(i * 3);
    }

    // 1. Ingest and route STANAG Dataplane packet
    printf("[TEST] Routing STANAG packet over Dataplane...\n");
    fflush(stdout);
    ok = transmit_dataplane_packet(&mux, STANAG_PROTO, 100, sample_payload);
    assert(ok == true);

    dataplane_packet_t rx;
    ok = receive_and_route_dataplane(&mux, &rx);
    assert(ok == true);
    assert(rx.protocol_type == STANAG_PROTO);
    assert(rx.sequence_counter == 100);
    printf("   ✓ STANAG packet routed and verified successfully.\n");
    fflush(stdout);

    // 2. Ingest and route DECNET Dataplane packet
    printf("[TEST] Routing DECNET packet over Dataplane...\n");
    fflush(stdout);
    ok = transmit_dataplane_packet(&mux, DECNET_PROTO, 200, sample_payload);
    assert(ok == true);

    ok = receive_and_route_dataplane(&mux, &rx);
    assert(ok == true);
    assert(rx.protocol_type == DECNET_PROTO);
    assert(rx.sequence_counter == 200);
    printf("   ✓ DECNET packet routed and verified successfully.\n");
    fflush(stdout);

    // 3. Sequence Monotonicity violation check (STANAG regression to sequence 99)
    printf("[TEST] Verifying sequence monotonicity enforcement (regression check)...\n");
    fflush(stdout);
    ok = transmit_dataplane_packet(&mux, STANAG_PROTO, 99, sample_payload);
    assert(ok == true);
    
    ok = receive_and_route_dataplane(&mux, &rx);
    assert(ok == false); // Must reject out-of-order packet
    printf("   ✓ Out-of-order sequence regression successfully rejected by Dataplane.\n");
    fflush(stdout);

    // 4. Checksum corruption check
    printf("[TEST] Verifying checksum validation (corruption check)...\n");
    fflush(stdout);
    ok = transmit_dataplane_packet(&mux, DECNET_PROTO, 201, sample_payload);
    assert(ok == true);

    // Simulating bit corruption in transmission line
    dataplane_packet_t corrupt_packet;
    read(mux.socket_fds[1], &corrupt_packet, sizeof(dataplane_packet_t));
    corrupt_packet.payload[0] ^= 0xFF; // Corrupt payload byte
    
    // Write corrupted packet back to verify receiver check
    write(mux.socket_fds[0], &corrupt_packet, sizeof(dataplane_packet_t));
    ok = receive_and_route_dataplane(&mux, &rx);
    assert(ok == false); // Must reject corrupted checksum
    printf("   ✓ Corrupted checksum payload successfully rejected by Dataplane.\n");
    fflush(stdout);

    close(mux.socket_fds[0]);
    close(mux.socket_fds[1]);

    printf("=============================================================\n");
    printf("DATAPLANE MULTIPLEXER VERIFICATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);

    return 0;
}
