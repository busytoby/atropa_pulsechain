#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_ALUS 4
#define BENCH_ITERATIONS 10000000 // 10 Million messages to get solid profile figures

// Simulated ALU state mapping
typedef struct {
    uint32_t alu_id;
    uint64_t registers[4];
    uint64_t defcon_latch;
    uint64_t fieldata_model;
    uint64_t range_value;
    bool is_active;
} SimulatedALU;

// Simulated XCOM Active Bus
typedef struct {
    SimulatedALU alus[MAX_ALUS];
    int alu_count;
    uint32_t active_transactions;
} XCOMActiveBus;

static inline void xplos_bus_init(XCOMActiveBus *bus) {
    memset(bus, 0, sizeof(XCOMActiveBus));
}

static inline int xplos_bus_register_alu(XCOMActiveBus *bus, uint32_t alu_id) {
    if (bus->alu_count >= MAX_ALUS) return -1;
    SimulatedALU *alu = &bus->alus[bus->alu_count++];
    alu->alu_id = alu_id;
    memset(alu->registers, 0, sizeof(alu->registers));
    alu->defcon_latch = 0;
    alu->fieldata_model = 100;
    alu->range_value = 0;
    alu->is_active = true;
    return 0;
}

// Inlined message routing for maximum throughput testing
static inline int xplos_xcom_route_active_message(XCOMActiveBus *bus, uint32_t src_alu_idx, uint32_t dst_alu_idx, uint16_t target_reg, uint64_t payload) {
    SimulatedALU *dst = &bus->alus[dst_alu_idx];
    bus->active_transactions++;

    // Routing branches
    if (target_reg == 0x4800) {
        dst->fieldata_model = payload;
    } else if (target_reg == 0x4080) {
        dst->defcon_latch = payload;
    } else if (target_reg == 0x4840) {
        dst->range_value = payload;
    } else {
        dst->registers[src_alu_idx % 4] = payload;
    }
    return 0;
}

int main(void) {
    printf("=============================================================\n");
    printf("xplos XCOM ACTIVE VM BUS PERFORMANCE PROFILE BENCHMARK\n");
    printf("=============================================================\n");

    XCOMActiveBus bus;
    xplos_bus_init(&bus);

    // Register 4 ALUs to simulate a quad-core active bus configuration
    for (uint32_t i = 0; i < 4; i++) {
        xplos_bus_register_alu(&bus, 100 + i);
    }

    printf("[BENCH] Starting execution loop: %d messages...\n", BENCH_ITERATIONS);

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // Main benchmark execution loop: routing packets round-robin among the 4 cores
    for (uint32_t i = 0; i < BENCH_ITERATIONS; i++) {
        uint32_t src = i % 4;
        uint32_t dst = (i + 1) % 4;
        // Alternate between different VM registers to exercise the decoder logic
        uint16_t reg = (i % 3 == 0) ? 0x4800 : ((i % 3 == 1) ? 0x4080 : 0x4840);
        
        xplos_xcom_route_active_message(&bus, src, dst, reg, i);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    double elapsed_sec = (end_time.tv_sec - start_time.tv_sec) + 
                         (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    double mtps = (BENCH_ITERATIONS / elapsed_sec) / 1e6; // Million Transactions Per Second
    double latency_ns = (elapsed_sec / BENCH_ITERATIONS) * 1e9; // Nanoseconds per message

    printf("\n--- PROFILE RESULTS ---\n");
    printf("  Elapsed Time   : %.4f seconds\n", elapsed_sec);
    printf("  Throughput     : %.2f Million Messages/sec (MTPS)\n", mtps);
    printf("  Average Latency: %.2f nanoseconds per transmission\n", latency_ns);
    printf("-------------------------\n");

    // Assert that the state was updated to verify loop wasn't optimized away
    assert(bus.active_transactions == BENCH_ITERATIONS);
    printf("   ✓ State verification passed successfully.\n");
    printf("=============================================================\n");

    return 0;
}
