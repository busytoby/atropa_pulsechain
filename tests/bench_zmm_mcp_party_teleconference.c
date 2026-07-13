#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>

#define NUM_PARTY_PEERS 6
#define BENCHMARK_PASSES 30000
#define PACKET_SIZE 512

typedef enum {
    PEER_IDLE = 0,
    PEER_ACTIVE = 1
} PeeringState;

typedef struct {
    uint32_t peer_id;
    PeeringState state;
    uint32_t packets_received;
    uint32_t packets_retransmitted;
} PartyPeer;

typedef struct {
    uint32_t src_peer_id;
    uint32_t dest_peer_id; // 0xFFFFFFFF = Party-Line Broadcast
    uint8_t payload[PACKET_SIZE];
    bool is_high_priority; // Active Peering voice/video vs Idle Peering stego-telemetry
} TeleconferencePacket;

// Process a packet routing event over the bridged coaxial ISDN party loop
void route_party_packet(PartyPeer *peers, size_t peer_count, TeleconferencePacket *packet, uint64_t *processed_bytes) {
    for (size_t i = 0; i < peer_count; i++) {
        // Skip sender retransmitting to itself
        if (peers[i].peer_id == packet->src_peer_id) {
            continue;
        }

        // Verify destination (Party broadcast or specific peer unicast)
        if (packet->dest_peer_id == 0xFFFFFFFF || packet->dest_peer_id == peers[i].peer_id) {
            peers[i].packets_received++;
            *processed_bytes += PACKET_SIZE;

            // Retransmit logic for active peering nodes (peering forward)
            if (peers[i].state == PEER_ACTIVE) {
                peers[i].packets_retransmitted++;
                *processed_bytes += PACKET_SIZE;
            }
        }
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COAXIAL ISDN PARTY-LINE TELECONFERENCE BENCHMARK\n");
    printf("=============================================================\n");

    // Initialize 6 party-line nodes (3 active voice/video nodes, 3 idle stego-telemetry nodes)
    PartyPeer peers[NUM_PARTY_PEERS];
    for (uint32_t i = 0; i < NUM_PARTY_PEERS; i++) {
        peers[i].peer_id = 100 + i;
        peers[i].state = (i % 2 == 0) ? PEER_ACTIVE : PEER_IDLE;
        peers[i].packets_received = 0;
        peers[i].packets_retransmitted = 0;
        
        printf("   Node [%u] status initialized: %s\n", 
               peers[i].peer_id, (peers[i].state == PEER_ACTIVE) ? "ACTIVE (Voice/Video)" : "IDLE (Stego-Telemetry)");
    }
    printf("\n");

    TeleconferencePacket packet;
    memset(&packet.payload, 0xEE, sizeof(packet.payload));
    
    uint64_t total_processed_bytes = 0;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // 1. Benchmark Active Peering broadcast passes (Voice/Video)
    packet.src_peer_id = 100; // Node 100 (Active)
    packet.dest_peer_id = 0xFFFFFFFF; // Broadcast
    packet.is_high_priority = true;
    for (int i = 0; i < BENCHMARK_PASSES; i++) {
        route_party_packet(peers, NUM_PARTY_PEERS, &packet, &total_processed_bytes);
    }

    // 2. Benchmark Idle Peering unicast passes (Background telemetry from Node 101 to Node 102)
    packet.src_peer_id = 101; // Node 101 (Idle)
    packet.dest_peer_id = 102; // Node 102 (Active)
    packet.is_high_priority = false;
    for (int i = 0; i < BENCHMARK_PASSES; i++) {
        route_party_packet(peers, NUM_PARTY_PEERS, &packet, &total_processed_bytes);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double throughput = (total_processed_bytes / (1024.0 * 1024.0)) / elapsed;

    // Validate metrics
    for (int i = 0; i < NUM_PARTY_PEERS; i++) {
        if (peers[i].peer_id == 100) {
            // Sender of broadcast, recipient of nothing
            assert(peers[i].packets_received == 0);
        } else if (peers[i].peer_id == 102) {
            // Recipient of broadcast and unicast, active (retransmits)
            assert(peers[i].packets_received == BENCHMARK_PASSES * 2);
            assert(peers[i].packets_retransmitted == BENCHMARK_PASSES * 2);
        } else {
            // Other nodes receive only broadcast
            assert(peers[i].packets_received == BENCHMARK_PASSES);
        }
        printf("   Node [%u]: Recv = %u, Retransmit = %u\n", 
               peers[i].peer_id, peers[i].packets_received, peers[i].packets_retransmitted);
    }

    printf("\nBenchmark Results:\n");
    printf("   Peers Evaluated:      %d\n", NUM_PARTY_PEERS);
    printf("   Total Bytes Routed:    %lu bytes\n", total_processed_bytes);
    printf("   Elapsed Time:          %.4f seconds\n", elapsed);
    printf("   Peering Throughput:    %.2f MB/s\n", throughput);
    printf("=============================================================\n");
    printf("PARTY-LINE TELECONFERENCE BENCHMARK COMPLETE\n");
    printf("=============================================================\n");

    return 0;
}
