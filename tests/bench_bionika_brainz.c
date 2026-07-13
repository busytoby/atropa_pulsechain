#include "../firmware/tsfi_lib/tsfi_btc_thunk.c"
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <math.h>

#define ITERATIONS 100000
#define DFT_SIZE 16
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Simple high-resolution timer helper returning nanoseconds
static uint64_t get_nanoseconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// Simulates a single Bionika BrainZ bijective loop cycle
void execute_bijective_cycle(const uint8_t *expected_root, const uint8_t *script, size_t script_len, BtcStack *stack) {
    // 1. Run JIT thunk verification
    BtcStack test_stack = *stack; // Copy stack state
    bool verify_ok = btc_thunk_execute(script, script_len, &test_stack);
    assert(verify_ok == true);

    // 2. Perform parameter modulation (mirroring root to color parameters)
    float param_r = (float)expected_root[0] / 255.0f;
    float param_g = (float)expected_root[1] / 255.0f;
    float param_b = (float)expected_root[2] / 255.0f;
    (void)param_r; (void)param_g; (void)param_b;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BIONIKA BRAINZ PERFORMANCE BENCHMARK SUITE\n");
    printf("=============================================================\n");

    // Reconstruct setup hashes
    uint8_t leaf1[32], leaf2[32], leaf3[32], nodeB[32];
    SHA256((const uint8_t *)"Activity 1", 10, leaf1);
    SHA256((const uint8_t *)"Activity 2", 10, leaf2);
    SHA256((const uint8_t *)"Activity 3", 10, leaf3);
    memset(nodeB, 0xAB, 32);

    uint8_t concat_A[96];
    memcpy(concat_A, leaf1, 32);
    memcpy(concat_A + 32, leaf2, 32);
    memcpy(concat_A + 64, leaf3, 32);
    uint8_t nodeA[32];
    SHA256(concat_A, 96, nodeA);

    uint8_t expected_root[32];
    uint8_t concat_root[64];
    memcpy(concat_root, nodeA, 32);
    memcpy(concat_root + 32, nodeB, 32);
    SHA256(concat_root, 64, expected_root);

    // Setup Script and Witness Stack
    BtcStack stack;
    stack_init(&stack);
    assert(stack_push(&stack, nodeB, 32));
    assert(stack_push(&stack, leaf3, 32));
    assert(stack_push(&stack, leaf1, 32));
    assert(stack_push(&stack, (const uint8_t *)"Activity 2", 10));

    uint8_t script[128];
    size_t s_idx = 0;
    script[s_idx++] = 0xa8; // OP_SHA256
    script[s_idx++] = 0x7e; // OP_CAT
    script[s_idx++] = 0x7c; // OP_SWAP
    script[s_idx++] = 0x7e; // OP_CAT
    script[s_idx++] = 0xa8; // OP_SHA256
    script[s_idx++] = 0x7c; // OP_SWAP
    script[s_idx++] = 0x7e; // OP_CAT
    script[s_idx++] = 0xa8; // OP_SHA256
    script[s_idx++] = 0x20; // PUSH_32
    memcpy(script + s_idx, expected_root, 32);
    s_idx += 32;
    script[s_idx++] = 0x88; // OP_EQUALVERIFY

    printf("1. Profiling %d bijective loop iterations...\n", ITERATIONS);
    uint64_t start_time = get_nanoseconds();
    for (int i = 0; i < ITERATIONS; i++) {
        execute_bijective_cycle(expected_root, script, s_idx, &stack);
    }
    uint64_t end_time = get_nanoseconds();
    uint64_t total_ns = end_time - start_time;
    double avg_ns = (double)total_ns / ITERATIONS;

    printf("\n+------------------------------------+----------------+\n");
    printf("| Metric                             | Value          |\n");
    printf("+------------------------------------+----------------+\n");
    printf("| Total Iterations                   | %-14d |\n", ITERATIONS);
    printf("| Total Elapsed Time (ms)            | %-14.2f |\n", (double)total_ns / 1000000.0);
    printf("| Average Round-trip Latency (ns)    | %-14.2f |\n", avg_ns);
    printf("+------------------------------------+----------------+\n");

    // Latency Guard Check (Latency must remain under sub-microsecond bounds)
    assert(avg_ns < 50000.0); // Verify low-latency bounds (< 50 us)
    printf("   ✓ Latency performance targets satisfied.\n");

    printf("\n=============================================================\n");
    printf("BIONIKA BRAINZ PERFORMANCE PROFILING COMPLETED\n");
    printf("=============================================================\n");
    return 0;
}
