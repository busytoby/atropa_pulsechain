#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>

#define BENCH_ITERATIONS 5000000 // 5 Million iterations per access path to measure clearly
#define COAXIAL_BUS_BASE 0x4000
#define WMQ_BASE 0x4800

typedef struct {
    uint8_t memory[0x10000];
} SimulatedVMContext;

// Access Method 1: Direct Memory-Mapped Coaxial Gating (Peek/Poke)
static inline void alu_access_direct_write(SimulatedVMContext *ctx, uint32_t v_reg, uint64_t val) {
    uint32_t offset = COAXIAL_BUS_BASE + (v_reg * 32);
    memcpy(&ctx->memory[offset], &val, sizeof(uint64_t));
}

static inline uint64_t alu_access_direct_read(SimulatedVMContext *ctx, uint32_t v_reg) {
    uint32_t offset = COAXIAL_BUS_BASE + (v_reg * 32);
    uint64_t val = 0;
    memcpy(&val, &ctx->memory[offset], sizeof(uint64_t));
    return val;
}

// Access Method 2: Selector Dynamic Dispatch (ABI)
static inline void alu_access_dispatch_write(SimulatedVMContext *ctx, uint32_t v_reg, uint64_t val) {
    if (v_reg == 0) return;
    alu_access_direct_write(ctx, v_reg, val);
}

static inline uint64_t alu_access_dispatch_read(SimulatedVMContext *ctx, uint32_t v_reg) {
    if (v_reg == 0) return 0;
    return alu_access_direct_read(ctx, v_reg);
}

// Access Method 3: WinchesterMQ Synchronization Routing (WMQ)
static inline void alu_access_wmq_sync(SimulatedVMContext *ctx, uint32_t target_core_id, uint32_t v_reg, uint64_t val) {
    memcpy(&ctx->memory[WMQ_BASE], &target_core_id, sizeof(uint32_t));
    memcpy(&ctx->memory[WMQ_BASE + 0x20], &v_reg, sizeof(uint32_t));
    memcpy(&ctx->memory[WMQ_BASE + 0x40], &val, sizeof(uint64_t));
}

int main(void) {
    printf("=============================================================\n");
    printf("xplos ALU-TO-RAU ACCESS METHODS PERFORMANCE BENCHMARK\n");
    printf("=============================================================\n");

    SimulatedVMContext ctx;
    memset(&ctx, 0, sizeof(SimulatedVMContext));

    struct timespec start, end;
    double elapsed;

    // --- PHASE 1: Direct Coaxial Gating (Memory-Mapped) ---
    printf("[BENCH] Phase 1: Direct Coaxial Gating (Peek/Poke) (%d ops)...\n", BENCH_ITERATIONS);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (uint32_t i = 0; i < BENCH_ITERATIONS; i++) {
        uint32_t v_reg = (i % 31) + 1;
        alu_access_direct_write(&ctx, v_reg, i);
        volatile uint64_t val = alu_access_direct_read(&ctx, v_reg);
        (void)val;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("  -> Direct Coaxial Throughput: %.2f MOPS | Latency: %.2f ns\n\n", 
           (BENCH_ITERATIONS / elapsed) / 1e6, (elapsed / BENCH_ITERATIONS) * 1e9);

    // --- PHASE 2: Selector Dynamic Dispatch (ABI) ---
    printf("[BENCH] Phase 2: Selector Dynamic Dispatch (ABI) (%d ops)...\n", BENCH_ITERATIONS);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (uint32_t i = 0; i < BENCH_ITERATIONS; i++) {
        uint32_t v_reg = i % 32;
        alu_access_dispatch_write(&ctx, v_reg, i);
        volatile uint64_t val = alu_access_dispatch_read(&ctx, v_reg);
        (void)val;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("  -> Dynamic Dispatch Throughput: %.2f MOPS | Latency: %.2f ns\n\n", 
           (BENCH_ITERATIONS / elapsed) / 1e6, (elapsed / BENCH_ITERATIONS) * 1e9);

    // --- PHASE 3: WinchesterMQ Synchronization Routing ---
    printf("[BENCH] Phase 3: WinchesterMQ Synchronization Routing (WMQ) (%d ops)...\n", BENCH_ITERATIONS);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (uint32_t i = 0; i < BENCH_ITERATIONS; i++) {
        uint32_t v_reg = (i % 31) + 1;
        alu_access_wmq_sync(&ctx, 2, v_reg, i);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("  -> WMQ Sync Routing Throughput: %.2f MOPS | Latency: %.2f ns\n\n", 
           (BENCH_ITERATIONS / elapsed) / 1e6, (elapsed / BENCH_ITERATIONS) * 1e9);

    printf("=============================================================\n");
    printf("ALU-TO-RAU ACCESS METHODS BENCHMARK COMPLETE\n");
    printf("=============================================================\n");

    return 0;
}
