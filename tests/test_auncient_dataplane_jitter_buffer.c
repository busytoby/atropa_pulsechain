#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define BUFFER_CAPACITY 8
#define PAYLOAD_SIZE 64
#define ALPHA 0.2 // Smoothing factor for running average deviation

typedef struct {
    uint64_t sequence_counter;
    uint8_t payload[PAYLOAD_SIZE];
    double send_time; // Simulates transmission timestamp
} jitter_packet_t;

typedef struct {
    jitter_packet_t queue[BUFFER_CAPACITY];
    int count;
    double target_delay;
    double avg_deviation;
} dataplane_jitter_buffer_t;

static void init_jitter_buffer(dataplane_jitter_buffer_t *jb) {
    jb->count = 0;
    jb->target_delay = 0.1; // 100ms initial target delay
    jb->avg_deviation = 0.0;
}

// Ingests packet and dynamically updates delay window feedback
static void ingest_packet(dataplane_jitter_buffer_t *jb, const jitter_packet_t *pkt, double current_time) {
    if (jb->count >= BUFFER_CAPACITY) {
        // Queue full, drop oldest or overwrite (simple sliding window drop for simulation)
        for (int i = 1; i < BUFFER_CAPACITY; i++) {
            jb->queue[i - 1] = jb->queue[i];
        }
        jb->count = BUFFER_CAPACITY - 1;
    }

    jb->queue[jb->count++] = *pkt;

    // Calculate packet arrival phase delay
    double delay = current_time - pkt->send_time;

    // Update running average deviation: (1 - alpha) * prev + alpha * |delay - target|
    double error = fabs(delay - jb->target_delay);
    jb->avg_deviation = (1.0 - ALPHA) * jb->avg_deviation + ALPHA * error;

    // Dynamic delay adjustment (PLL feedback lock)
    if (jb->avg_deviation > 0.02) {
        jb->target_delay += 0.025; // Increase buffer window to absorb jitter
    } else if (jb->avg_deviation < 0.005 && jb->target_delay > 0.05) {
        jb->target_delay -= 0.005; // Decrease window to minimize latency
    }
}

// Releases the oldest packet if its buffered duration meets the target delay
static bool release_packet(dataplane_jitter_buffer_t *jb, double current_time, jitter_packet_t *out_pkt) {
    if (jb->count == 0) {
        return false;
    }

    // Check if oldest packet has been buffered long enough to satisfy target delay
    double buffered_duration = current_time - jb->queue[0].send_time;
    if (buffered_duration >= jb->target_delay) {
        *out_pkt = jb->queue[0];

        // Shift queue
        for (int i = 1; i < jb->count; i++) {
            jb->queue[i - 1] = jb->queue[i];
        }
        jb->count--;
        return true;
    }

    return false;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT DATAPLANE JITTER BUFFER (DLL) VERIFICATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    dataplane_jitter_buffer_t jb;
    init_jitter_buffer(&jb);

    double current_time = 0.0;
    jitter_packet_t pkt;
    memset(pkt.payload, 0, PAYLOAD_SIZE);

    // 1. Ingest packets with stable arrival intervals (low jitter)
    printf("[TEST] Ingesting packet stream with stable arrival intervals...\n");
    fflush(stdout);
    
    for (int i = 0; i < 5; i++) {
        pkt.sequence_counter = i + 1;
        pkt.send_time = current_time;
        
        current_time += 0.105; // ~105ms arrival time (very stable, 5ms error)
        ingest_packet(&jb, &pkt, current_time);
    }

    // Assert target delay has converged or remained stable
    assert(jb.target_delay < 0.15);
    printf("   ✓ Buffer target delay stabilized at: %.4fs (dev: %.4fs).\n", jb.target_delay, jb.avg_deviation);
    fflush(stdout);

    // 2. Ingest packets with high jitter (phase noise spike)
    printf("[TEST] Injecting high jitter phase noise spike...\n");
    fflush(stdout);

    // Packet arrives with a huge delay spike (350ms delay)
    pkt.sequence_counter = 6;
    pkt.send_time = current_time;
    current_time += 0.350; 
    ingest_packet(&jb, &pkt, current_time);

    // Buffer should dynamically adapt and increase its target delay window
    assert(jb.target_delay > 0.10);
    printf("   ✓ Buffer dynamically adjusted target delay to: %.4fs due to jitter.\n", jb.target_delay);
    fflush(stdout);

    // 3. Release packets and verify smoothed output intervals
    printf("[TEST] Releasing packets and checking playback rate consistency...\n");
    fflush(stdout);
    
    jitter_packet_t rx;
    int released_count = 0;
    double last_release_time = 0.0;

    // Run clock forward to release buffered packets
    for (int step = 0; step < 20; step++) {
        current_time += 0.05; // Move time forward in 50ms increments
        if (release_packet(&jb, current_time, &rx)) {
            if (released_count > 0) {
                double interval = current_time - last_release_time;
                // Output intervals must be smoothed compared to the 350ms arrival spike
                assert(interval < 0.25);
            }
            last_release_time = current_time;
            released_count++;
        }
    }

    assert(released_count > 0);
    printf("   ✓ Released %d packets. Playback intervals smoothed successfully.\n", released_count);
    fflush(stdout);

    printf("=============================================================\n");
    printf("JITTER BUFFER VERIFICATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);

    return 0;
}
