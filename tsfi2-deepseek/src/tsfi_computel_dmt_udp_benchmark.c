#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/*
 * Auncient Computel DMT-over-UDP Benchmark
 * 
 * Simulates Discrete Multi-Tone sub-channel allocation over multiple UDP ports.
 * Dynamically profiles loss rates (SNR) and redirects traffic based on priority.
 */

#define NUM_UDP_BINS 4
#define BENCHMARK_ITERATIONS 1000

typedef struct {
    uint16_t port_offset;
    float simulated_loss_rate;
    uint32_t packets_sent;
    uint32_t packets_lost;
    float calculated_snr; // 1.0 = clean, 0.0 = completely blocked
} UdpBin;

typedef enum {
    PRIORITY_HIGH, // Critical key/telemetry data
    PRIORITY_LOW   // Non-critical coordinate stream
} PayloadPriority;

typedef struct {
    UdpBin bins[NUM_UDP_BINS];
    uint32_t high_priority_delivered;
    uint32_t high_priority_lost;
    uint32_t low_priority_delivered;
    uint32_t low_priority_lost;
} DmtUdpBenchmark;

void init_dmt_udp_benchmark(DmtUdpBenchmark *bench) {
    if (!bench) return;
    bench->high_priority_delivered = 0;
    bench->high_priority_lost = 0;
    bench->low_priority_delivered = 0;
    bench->low_priority_lost = 0;

    // Initialize 4 UDP virtual ports/bins with simulated network loss
    bench->bins[0] = (UdpBin){8000, 0.02f, 0, 0, 1.0f}; // Clean bin
    bench->bins[1] = (UdpBin){8001, 0.15f, 0, 0, 1.0f}; // Mild noise
    bench->bins[2] = (UdpBin){8002, 0.60f, 0, 0, 1.0f}; // High noise
    bench->bins[3] = (UdpBin){8003, 0.95f, 0, 0, 1.0f}; // Blocked bin
}

/* Recalculates virtual SNR values based on packet success statistics */
void update_bin_snr(UdpBin *bin) {
    if (bin->packets_sent == 0) return;
    float loss = (float)bin->packets_lost / bin->packets_sent;
    bin->calculated_snr = 1.0f - loss;
}

/* Routes payload dynamically using DMT bit-loading priority matching */
void transmit_dmt_payload(DmtUdpBenchmark *bench, PayloadPriority priority) {
    if (!bench) return;

    // Find the best available bin index based on calculated SNR
    int target_bin_idx = 0;
    float best_snr = -1.0f;

    if (priority == PRIORITY_HIGH) {
        // High priority must route to the highest SNR bin
        for (int i = 0; i < NUM_UDP_BINS; i++) {
            if (bench->bins[i].calculated_snr > best_snr) {
                best_snr = bench->bins[i].calculated_snr;
                target_bin_idx = i;
            }
        }
    } else {
        // Low priority routes to lower SNR bins to preserve high-quality lines
        float worst_snr = 2.0f;
        for (int i = 0; i < NUM_UDP_BINS; i++) {
            if (bench->bins[i].calculated_snr < worst_snr) {
                worst_snr = bench->bins[i].calculated_snr;
                target_bin_idx = i;
            }
        }
    }

    UdpBin *target_bin = &bench->bins[target_bin_idx];
    target_bin->packets_sent++;

    // Simulate network drop based on bin properties
    float r = (float)rand() / RAND_MAX;
    bool dropped = (r < target_bin->simulated_loss_rate);

    if (dropped) {
        target_bin->packets_lost++;
        if (priority == PRIORITY_HIGH) {
            bench->high_priority_lost++;
        } else {
            bench->low_priority_lost++;
        }
    } else {
        if (priority == PRIORITY_HIGH) {
            bench->high_priority_delivered++;
        } else {
            bench->low_priority_delivered++;
        }
    }

    // Adaptively update SNR profiles
    update_bin_snr(target_bin);
}
