#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>

#define BENCH_ITERATIONS 5000000

typedef struct {
    uint64_t simulated_storage[0x10000]; // Simulated EVM Storage
    uint8_t memory[0x10000];             // 64KB VM Memory
} SimulatedVMContext;

// Method 1: Storage-Backed Register Access (Before)
static inline void cpu_reg_storage_write(SimulatedVMContext *ctx, uint32_t addr, uint64_t val) {
    ctx->simulated_storage[addr] = val;
}

static inline uint64_t cpu_reg_storage_read(SimulatedVMContext *ctx, uint32_t addr) {
    return ctx->simulated_storage[addr];
}

// Method 2: Stateless Coaxial RAU Register Access (After)
static inline void cpu_reg_coaxial_write(SimulatedVMContext *ctx, uint32_t addr, uint64_t val) {
    uint32_t offset = 0x4000 + ((addr - 0x7f) * 32);
    memcpy(&ctx->memory[offset], &val, sizeof(uint64_t));
}

static inline uint64_t cpu_reg_coaxial_read(SimulatedVMContext *ctx, uint32_t addr) {
    uint32_t offset = 0x4000 + ((addr - 0x7f) * 32);
    uint64_t val = 0;
    memcpy(&val, &ctx->memory[offset], sizeof(uint64_t));
    return val;
}

int main(void) {
    printf("=============================================================\n");
    printf("FOLKLORE CPU REGISTERS ACCESS METHOD PERFORMANCE BENCHMARK\n");
    printf("=============================================================\n");

    SimulatedVMContext ctx;
    memset(&ctx, 0, sizeof(SimulatedVMContext));

    struct timespec start, end;
    double elapsed;

    // --- PHASE 1: Storage-Backed Registers (Before) ---
    printf("[BENCH] Phase 1: Storage-Backed Register Access (%d ops)...\n", BENCH_ITERATIONS);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (uint32_t i = 0; i < BENCH_ITERATIONS; i++) {
        uint32_t addr = 0x80 + (i % 6);
        cpu_reg_storage_write(&ctx, addr, i);
        volatile uint64_t val = cpu_reg_storage_read(&ctx, addr);
        (void)val;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("  -> Storage-Backed Throughput: %.2f MOPS | Latency: %.2f ns\n\n", 
           (BENCH_ITERATIONS / elapsed) / 1e6, (elapsed / BENCH_ITERATIONS) * 1e9);

    // --- PHASE 2: Stateless Coaxial RAU Registers (After) ---
    printf("[BENCH] Phase 2: Stateless Coaxial RAU Register Access (%d ops)...\n", BENCH_ITERATIONS);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (uint32_t i = 0; i < BENCH_ITERATIONS; i++) {
        uint32_t addr = 0x80 + (i % 6);
        cpu_reg_coaxial_write(&ctx, addr, i);
        volatile uint64_t val = cpu_reg_coaxial_read(&ctx, addr);
        (void)val;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("  -> Stateless Coaxial Throughput: %.2f MOPS | Latency: %.2f ns\n\n", 
           (BENCH_ITERATIONS / elapsed) / 1e6, (elapsed / BENCH_ITERATIONS) * 1e9);

    printf("=============================================================\n");
    printf("FOLKLORE CPU REGISTERS BENCHMARK COMPLETE\n");
    printf("=============================================================\n");

    return 0;
}
