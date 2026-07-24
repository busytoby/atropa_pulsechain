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
#define VECTOR_BATCH_LIMIT 8
#define MAX_DATAPLANES 2

typedef struct {
    uint64_t sequence;
    uint32_t destination_port; // Target interface ID (1 or 2)
    uint8_t payload[PAYLOAD_SIZE];
} vpp_packet_t;

typedef struct {
    int rx_fd;
    int tx_fd;
    bool active;
    uint32_t port_id;
} vpp_dataplane_t;

typedef struct {
    vpp_dataplane_t dataplanes[MAX_DATAPLANES];
} vpp_context_t;

// Initialize VPP forwarding engine with two dataplane interfaces
static void vpp_init_forwarder(vpp_context_t *vpp, int dp1_rx, int dp1_tx, int dp2_rx, int dp2_tx) {
    vpp->dataplanes[0].rx_fd = dp1_rx;
    vpp->dataplanes[0].tx_fd = dp1_tx;
    vpp->dataplanes[0].active = true;
    vpp->dataplanes[0].port_id = 1;

    vpp->dataplanes[1].rx_fd = dp2_rx;
    vpp->dataplanes[1].tx_fd = dp2_tx;
    vpp->dataplanes[1].active = true;
    vpp->dataplanes[1].port_id = 2;
}

// Ingests packets from a specific source interface into a vector batch
static int vpp_ingest_from_port(vpp_dataplane_t *dp, vpp_packet_t *vector_batch) {
    int count = 0;
    for (int i = 0; i < VECTOR_BATCH_LIMIT; i++) {
        vpp_packet_t pkt;
        ssize_t bytes = recv(dp->rx_fd, &pkt, sizeof(vpp_packet_t), MSG_DONTWAIT);
        if (bytes == sizeof(vpp_packet_t)) {
            vector_batch[count++] = pkt;
        } else {
            break;
        }
    }
    return count;
}

// Graph Node: Classify and dispatch vector batch to target ports (Vector Forwarding Table)
static void vpp_node_classify_and_dispatch(vpp_context_t *vpp, const vpp_packet_t *vector_batch, int count) {
    // Reorder-buffers for destinations to maintain cache alignment
    vpp_packet_t port1_batch[VECTOR_BATCH_LIMIT];
    int port1_count = 0;

    vpp_packet_t port2_batch[VECTOR_BATCH_LIMIT];
    int port2_count = 0;

    // Single-pass vector classification
    for (int i = 0; i < count; i++) {
        if (vector_batch[i].destination_port == 1) {
            port1_batch[port1_count++] = vector_batch[i];
        } else if (vector_batch[i].destination_port == 2) {
            port2_batch[port2_count++] = vector_batch[i];
        }
    }

    // Bulk dispatch sub-vectors to target ports
    for (int i = 0; i < port1_count; i++) {
        write(vpp->dataplanes[0].tx_fd, &port1_batch[i], sizeof(vpp_packet_t));
    }
    for (int i = 0; i < port2_count; i++) {
        write(vpp->dataplanes[1].tx_fd, &port2_batch[i], sizeof(vpp_packet_t));
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT VPP MULTI-DATAPLANE VECTOR FORWARDING SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    int dp1_fds[2];
    int dp2_fds[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, dp1_fds) >= 0);
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, dp2_fds) >= 0);

    vpp_context_t vpp;
    vpp_init_forwarder(&vpp, dp1_fds[0], dp1_fds[0], dp2_fds[0], dp2_fds[0]);
    printf("   ✓ Registered Dataplane Port 1 and Port 2 in VPP Forwarding Table.\n");
    fflush(stdout);

    // 1. Queue input packets with mixed destination targets
    vpp_packet_t input[4];
    for (int i = 0; i < 4; i++) {
        input[i].sequence = i + 1;
        // Alternate targets: packets 1 and 3 target Port 2; packets 2 and 4 target Port 1
        input[i].destination_port = (i % 2 == 0) ? 2 : 1;
        memset(input[i].payload, (uint8_t)(i * 10), PAYLOAD_SIZE);
        
        // Write packets onto Port 1 input line (acting as ingest port)
        write(dp1_fds[1], &input[i], sizeof(vpp_packet_t));
    }

    // 2. Ingest packets as a vector from Port 1
    printf("[TEST] Ingesting packet vector batch...\n");
    fflush(stdout);
    vpp_packet_t vector[VECTOR_BATCH_LIMIT];
    int ingested = vpp_ingest_from_port(&(vpp.dataplanes[0]), vector);
    assert(ingested == 4);
    printf("   ✓ Ingested vector size: %d.\n", ingested);
    fflush(stdout);

    // 3. Classify and route sub-vectors in bulk
    printf("[TEST] Classifying and dispatching sub-vectors...\n");
    fflush(stdout);
    vpp_node_classify_and_dispatch(&vpp, vector, ingested);
    printf("   ✓ Packet vector successfully classified and switched.\n");
    fflush(stdout);

    // 4. Verify Port 1 received its target packets (sequences 2 and 4)
    printf("[TEST] Verifying Port 1 output stream...\n");
    fflush(stdout);
    vpp_packet_t rx_pkt;
    
    // Read first packet on Port 1
    read(dp1_fds[1], &rx_pkt, sizeof(vpp_packet_t));
    assert(rx_pkt.sequence == 2 && rx_pkt.destination_port == 1);
    
    // Read second packet on Port 1
    read(dp1_fds[1], &rx_pkt, sizeof(vpp_packet_t));
    assert(rx_pkt.sequence == 4 && rx_pkt.destination_port == 1);
    printf("   ✓ Port 1 stream valid.\n");
    fflush(stdout);

    // 5. Verify Port 2 received its target packets (sequences 1 and 3)
    printf("[TEST] Verifying Port 2 output stream...\n");
    fflush(stdout);
    
    // Read first packet on Port 2
    read(dp2_fds[1], &rx_pkt, sizeof(vpp_packet_t));
    assert(rx_pkt.sequence == 1 && rx_pkt.destination_port == 2);
    
    // Read second packet on Port 2
    read(dp2_fds[1], &rx_pkt, sizeof(vpp_packet_t));
    assert(rx_pkt.sequence == 3 && rx_pkt.destination_port == 2);
    printf("   ✓ Port 2 stream valid.\n");
    fflush(stdout);

    close(dp1_fds[0]);
    close(dp1_fds[1]);
    close(dp2_fds[0]);
    close(dp2_fds[1]);

    printf("=============================================================\n");
    printf("VPP MULTI-DATAPLANE ROUTING TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
