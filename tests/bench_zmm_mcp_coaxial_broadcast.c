#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>

#define NUM_RECEIVERS 8
#define BROADCAST_ADDR 0xFFFF
#define FRAME_SIZE 1024
#define NUM_FRAMES 50000

typedef struct {
    uint16_t address;
    uint32_t received_count;
    uint32_t filtered_count;
} ReceiverNode;

typedef struct {
    uint16_t dest_address;
    uint16_t src_address;
    uint8_t payload[FRAME_SIZE - 4];
} CoaxialFrame;

// Simulates transmission and measurement of one-to-many broadcast
void run_broadcast_benchmark(ReceiverNode *receivers, size_t rx_count, CoaxialFrame *frame, uint64_t *total_bytes_processed) {
    for (size_t i = 0; i < rx_count; i++) {
        // Validate destination address: match node address or global broadcast identifier
        if (frame->dest_address == BROADCAST_ADDR || frame->dest_address == receivers[i].address) {
            receivers[i].received_count++;
            *total_bytes_processed += FRAME_SIZE;
        } else {
            receivers[i].filtered_count++;
        }
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COAXIAL ISDN ONE-TO-MANY BROADCAST BENCHMARK\n");
    printf("=============================================================\n");

    // Initialize one sender and 8 receiver nodes
    ReceiverNode receivers[NUM_RECEIVERS];
    for (uint16_t i = 0; i < NUM_RECEIVERS; i++) {
        receivers[i].address = 0x1000 + i;
        receivers[i].received_count = 0;
        receivers[i].filtered_count = 0;
    }

    CoaxialFrame frame;
    memset(&frame, 0xAA, sizeof(frame));
    frame.src_address = 0x0100; // Host address

    uint64_t total_bytes = 0;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // 1. Send broadcast frames
    frame.dest_address = BROADCAST_ADDR;
    for (int i = 0; i < NUM_FRAMES; i++) {
        run_broadcast_benchmark(receivers, NUM_RECEIVERS, &frame, &total_bytes);
    }

    // 2. Send targeted/unicast frames (destined only to Node 2: address 0x1002)
    frame.dest_address = 0x1002;
    for (int i = 0; i < NUM_FRAMES; i++) {
        run_broadcast_benchmark(receivers, NUM_RECEIVERS, &frame, &total_bytes);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double throughput = (total_bytes / (1024.0 * 1024.0)) / elapsed;

    // Validate metrics
    for (int i = 0; i < NUM_RECEIVERS; i++) {
        if (receivers[i].address == 0x1002) {
            // Node 2 must receive both broadcast and targeted frames
            assert(receivers[i].received_count == NUM_FRAMES * 2);
            assert(receivers[i].filtered_count == 0);
        } else {
            // Other nodes receive only broadcast and filter targeted ones
            assert(receivers[i].received_count == NUM_FRAMES);
            assert(receivers[i].filtered_count == NUM_FRAMES);
        }
        printf("   Node [0x%X]: Received = %u, Filtered = %u\n", 
               receivers[i].address, receivers[i].received_count, receivers[i].filtered_count);
    }

    printf("\nBenchmark Results:\n");
    printf("   Total Frames Routed:  %d\n", NUM_FRAMES * 2);
    printf("   Receivers Audited:     %d\n", NUM_RECEIVERS);
    printf("   Total Bytes Processed: %lu bytes\n", total_bytes);
    printf("   Elapsed Time:          %.4f seconds\n", elapsed);
    printf("   Throughput Rate:       %.2f MB/s\n", throughput);
    printf("=============================================================\n");
    printf("ONE-TO-MANY BROADCAST BENCHMARK COMPLETE\n");
    printf("=============================================================\n");

    return 0;
}
