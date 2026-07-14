#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "../src/tsfi_computel_dmt_udp_benchmark.c"

int main(void) {
    printf("[BENCHMARK] Initializing Auncient Computel DMT-over-UDP Bit-Loading Benchmark...\n");

    DmtUdpBenchmark bench;
    init_dmt_udp_benchmark(&bench);

    // Seed rand for reproducible benchmark runs
    srand(12345);

    // Execute transmissions
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        transmit_dmt_payload(&bench, PRIORITY_HIGH);
        transmit_dmt_payload(&bench, PRIORITY_LOW);
    }

    uint32_t total_high = bench.high_priority_delivered + bench.high_priority_lost;
    uint32_t total_low = bench.low_priority_delivered + bench.low_priority_lost;

    float high_rate = (float)bench.high_priority_delivered / total_high * 100.0f;
    float low_rate = (float)bench.low_priority_delivered / total_low * 100.0f;

    printf("\n--- DMT-over-UDP Benchmark Results ---\n");
    printf("High Priority Packets: Sent %d | Delivered %d | Lost %d | Success Rate: %.2f%%\n",
           total_high, bench.high_priority_delivered, bench.high_priority_lost, high_rate);
    printf("Low Priority Packets:  Sent %d | Delivered %d | Lost %d | Success Rate: %.2f%%\n",
           total_low, bench.low_priority_delivered, bench.low_priority_lost, low_rate);
    printf("--------------------------------------\n\n");

    // High priority must route to clean bin (Port 8000: 2% simulated loss) -> Success rate > 95%
    assert(high_rate > 95.0f);
    
    // Low priority must route to worst bins (Port 8003: 95% simulated loss) -> Success rate < 10%
    assert(low_rate < 10.0f);

    printf("[SUCCESS] DMT bit-loading priority routing benchmarks executed successfully.\n");
    return 0;
}
