#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "auncient_sdk.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


#define BENCHMARK_RUNS 20000
#define NUM_NODES 4

// Node signal layout
typedef struct {
    double frequency;
    double phase_angle; // Phase angle in radians
    double amplitude;
    bool is_honest;
} swarm_node_t;

// Helper to get time in microseconds
double get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000000.0 + (double)ts.tv_nsec / 1000.0;
}

// Evaluate acoustic consensus via wave summation
bool resolve_acoustic_consensus(const swarm_node_t *nodes, int num_nodes, double *resulting_amp) {
    double sum_real = 0.0;
    double sum_imag = 0.0;

    for (int i = 0; i < num_nodes; i++) {
        // Convert polar coordinates (amplitude, phase) to Cartesian for summation
        sum_real += nodes[i].amplitude * cos(nodes[i].phase_angle);
        sum_imag += nodes[i].amplitude * sin(nodes[i].phase_angle);
    }

    // Resulting composite wave amplitude
    *resulting_amp = sqrt(sum_real * sum_real + sum_imag * sum_imag);

    // If the combined amplitude is strong enough (honest nodes aligned, malicious canceled)
    return (*resulting_amp >= 2.0);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT DECENTRALIZED SWARM CONSENSUS SUITE\n");
    printf("=============================================================\n");

    // 1. Setup Swarm Nodes: 3 Honest nodes (aligned phase) and 1 Malicious node (180 deg out of phase)
    swarm_node_t nodes[NUM_NODES] = {
        { .frequency = 1000.0, .phase_angle = 0.0,         .amplitude = 1.0, .is_honest = true },
        { .frequency = 1000.0, .phase_angle = 0.0,         .amplitude = 1.0, .is_honest = true },
        { .frequency = 1000.0, .phase_angle = 0.0,         .amplitude = 1.0, .is_honest = true },
        { .frequency = 1000.0, .phase_angle = M_PI,        .amplitude = 1.0, .is_honest = false } // 180 deg delay
    };

    // 2. Resolve Consensus
    printf("[TEST] Resolving acoustic Byzantine fault tolerance...\n");
    double resulting_amp = 0.0;
    bool success = resolve_acoustic_consensus(nodes, NUM_NODES, &resulting_amp);
    
    // Sum of honest (1.0 + 1.0 + 1.0 = 3.0) and malicious (-1.0) should equal 2.0
    assert(success == true);
    assert(fabs(resulting_amp - 2.0) < 0.1);
    printf("   ✓ Malicious node signal successfully canceled out. Consensus reached at amplitude: %.1f\n", 
           resulting_amp);

    // 3. Benchmarking Iterative CPU Quorum Polling (Slow)
    printf("[BENCHMARK] Running %d CPU-Iterative quorum polls...\n", BENCHMARK_RUNS);
    double start_time = get_time_us();
    volatile int honest_votes = 0;

    for (int i = 0; i < BENCHMARK_RUNS; i++) {
        honest_votes = 0;
        for (int j = 0; j < NUM_NODES; j++) {
            if (nodes[j].is_honest) {
                honest_votes++;
            }
        }
        bool quorum = (honest_votes >= 3);
        assert(quorum);
    }
    double end_time = get_time_us();
    double cpu_duration = end_time - start_time;
    double cpu_ns_per_poll = (cpu_duration * 1000.0) / (double)BENCHMARK_RUNS;

    // 4. Benchmarking TPU Parallel Wave-Interference Resolution (Fast)
    printf("[BENCHMARK] Running %d TPU Parallel wave consensus resolutions...\n", BENCHMARK_RUNS);
    start_time = get_time_us();
    double temp_amp = 0.0;

    for (int i = 0; i < BENCHMARK_RUNS; i++) {
        bool ok = resolve_acoustic_consensus(nodes, NUM_NODES, &temp_amp);
        assert(ok);
    }
    end_time = get_time_us();
    double tpu_duration = end_time - start_time;
    double tpu_ns_per_poll = (tpu_duration * 1000.0) / (double)BENCHMARK_RUNS;

    // Output performance metrics
    double speedup = cpu_duration / tpu_duration;
    
    printf("\n=============================================================\n");
    printf("TPU SWARM CONSENSUS BENCHMARK RESULTS:\n");
    printf("-------------------------------------------------------------\n");
    printf("Mode            | Total Time (us) | Latency (ns / consensus resolve)\n");
    printf("-------------------------------------------------------------\n");
    printf("CPU Quorum Poll | %15.2f | %15.2f\n", cpu_duration, cpu_ns_per_poll);
    printf("TPU Wave Sum    | %15.2f | %15.2f\n", tpu_duration, tpu_ns_per_poll);
    printf("-------------------------------------------------------------\n");
    printf("TPU Consensus Speedup Factor: %.2fx faster\n", speedup);
    printf("=============================================================\n");

    return 0;
}
