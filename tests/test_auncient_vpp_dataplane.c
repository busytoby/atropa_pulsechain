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
#define VECTOR_BATCH_LIMIT 4

typedef struct {
    uint64_t sequence;
    uint16_t checksum;
    uint8_t payload[PAYLOAD_SIZE];
} vpp_packet_t;

typedef struct {
    int rx_fd;
    int tx_fd;
    bool active;
    uint64_t interface_id;
} vpp_dataplane_t;

typedef struct {
    vpp_dataplane_t active_dataplane;
    bool running;
} vpp_context_t;

// Simple checksum generator
static uint16_t compute_checksum(const uint8_t *data, size_t len) {
    uint16_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

// Initialize VPP context with exactly one active Dataplane interface
static bool vpp_init_context(vpp_context_t *vpp, int rx_fd, int tx_fd, uint64_t iface_id) {
    vpp->active_dataplane.rx_fd = rx_fd;
    vpp->active_dataplane.tx_fd = tx_fd;
    vpp->active_dataplane.active = true;
    vpp->active_dataplane.interface_id = iface_id;
    vpp->running = true;
    return true;
}

// Graph Node 1: Ingests packets from Dataplane into Vector batch
static int vpp_node_ingest(vpp_context_t *vpp, vpp_packet_t *vector_batch) {
    int packets_ingested = 0;
    
    // Poll non-blocking to accumulate vector batch
    for (int i = 0; i < VECTOR_BATCH_LIMIT; i++) {
        vpp_packet_t pkt;
        ssize_t bytes = recv(vpp->active_dataplane.rx_fd, &pkt, sizeof(vpp_packet_t), MSG_DONTWAIT);
        if (bytes == sizeof(vpp_packet_t)) {
            vector_batch[packets_ingested++] = pkt;
        } else {
            break; // No more packets available in this poll window
        }
    }
    
    return packets_ingested;
}

// Graph Node 2: Validate Vector checksums in a single loop (vectorization friendly)
static int vpp_node_verify(vpp_packet_t *vector_batch, int count) {
    int valid_count = 0;
    for (int i = 0; i < count; i++) {
        uint16_t computed = compute_checksum(vector_batch[i].payload, PAYLOAD_SIZE);
        if (computed == vector_batch[i].checksum) {
            if (i != valid_count) {
                vector_batch[valid_count] = vector_batch[i]; // Compact valid packets
            }
            valid_count++;
        }
    }
    return valid_count;
}

// Graph Node 3: Process and Route valid vector batch
static int vpp_node_route(vpp_context_t *vpp, const vpp_packet_t *vector_batch, int count) {
    int routed_count = 0;
    for (int i = 0; i < count; i++) {
        // Echo packet back to output channel
        ssize_t sent = write(vpp->active_dataplane.tx_fd, &vector_batch[i], sizeof(vpp_packet_t));
        if (sent == sizeof(vpp_packet_t)) {
            routed_count++;
        }
    }
    return routed_count;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT VPP DATAPLANE VECTOR SWITCHING SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    int fds[2];
    bool ok = (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) >= 0);
    assert(ok == true);

    vpp_context_t vpp;
    ok = vpp_init_context(&vpp, fds[0], fds[0], 0xF001);
    assert(ok == true);
    printf("   ✓ VPP context initialized with one active Dataplane (ID: 0xF001).\n");
    fflush(stdout);

    // 1. Prepare packet batch data
    uint8_t payload[PAYLOAD_SIZE];
    memset(payload, 0x5A, PAYLOAD_SIZE);
    
    vpp_packet_t input_packets[3];
    for (int i = 0; i < 3; i++) {
        input_packets[i].sequence = i + 1;
        input_packets[i].checksum = compute_checksum(payload, PAYLOAD_SIZE);
        memcpy(input_packets[i].payload, payload, PAYLOAD_SIZE);
        
        // Write packets directly to dataplane input
        write(fds[1], &input_packets[i], sizeof(vpp_packet_t));
    }

    // 2. Execute VPP Graph Node Pipeline
    printf("[TEST] Executing VPP graph node pipeline...\n");
    fflush(stdout);

    vpp_packet_t vector[VECTOR_BATCH_LIMIT];
    
    // Step A: Ingest into vector
    int ingested = vpp_node_ingest(&vpp, vector);
    assert(ingested == 3);
    printf("   ✓ Node Ingest: Processed batch vector of size %d.\n", ingested);
    fflush(stdout);

    // Step B: Verify vector checksums
    int verified = vpp_node_verify(vector, ingested);
    assert(verified == 3);
    printf("   ✓ Node Verify: Validated checksums for %d packets.\n", verified);
    fflush(stdout);

    // Step C: Route vector packets
    int routed = vpp_node_route(&vpp, vector, verified);
    assert(routed == 3);
    printf("   ✓ Node Route: Successfully switched %d packets.\n", routed);
    fflush(stdout);

    // 3. Verify output on receiver side
    for (int i = 0; i < 3; i++) {
        vpp_packet_t out_pkt;
        read(fds[1], &out_pkt, sizeof(vpp_packet_t));
        assert(out_pkt.sequence == (uint64_t)(i + 1));
    }
    printf("   ✓ Output channels successfully verified.\n");
    fflush(stdout);

    close(fds[0]);
    close(fds[1]);

    printf("=============================================================\n");
    printf("VPP DATAPLANE SUITE VERIFICATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
