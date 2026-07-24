#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define BENCH_RUNS 200000

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
static void sha256(const void *data, size_t len, uint8_t *out) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(out, &ctx);
}
#pragma GCC diagnostic pop

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// 2-3 Merkle Tree Node
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// Helper to create leaf node
static TwoThreeNode* create_leaf(uint32_t key1, const char *val1) {
    TwoThreeNode *node = (TwoThreeNode*)calloc(1, sizeof(TwoThreeNode));
    node->is_leaf = true;
    node->num_keys = 1;
    node->keys[0] = key1;
    strcpy(node->values[0], val1);
    
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
    sha256(temp, len, node->node_hash);
    return node;
}

// -------------------------------------------------------------
// Benchmark Test Runner
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PL/EXUS ALU SUBSYSTEM BENCHMARK PERFORMANCE SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *root = create_leaf(700, "COS:0.00,SIN:0.00");
    volatile uint32_t target_key = 700;
    volatile uint8_t tag = 0x01; // TAG_KERNEL

    uint8_t temp[256];
    uint8_t current_verify[HASH_SIZE];
    uint32_t accumulated_conductions = 0;

    // -------------------------------------------------------------
    // Benchmark 1: Gated 2-3 Tree BASED Pointer Dereference Latency
    // -------------------------------------------------------------
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // Latency Guard Gate loop (No prints inside hot path)
    for (int i = 0; i < BENCH_RUNS; i++) {
        // Simulating BASED pointer lookup and validation
        if (root->keys[0] == target_key && tag == 0x01) {
            int len = snprintf((char*)temp, sizeof(temp), "%u:%s", root->keys[0], root->values[0]);
            sha256(temp, len, current_verify);
            accumulated_conductions++;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    double total_ns = (double)(end_time.tv_sec - start_time.tv_sec) * 1e9 +
                      (double)(end_time.tv_nsec - start_time.tv_nsec);
    double latency_per_run = total_ns / BENCH_RUNS;

    printf("[BENCHMARK 1] BASED Pointer Dereference & Verification:\n");
    printf("   Runs: %d\n", BENCH_RUNS);
    printf("   Total Time: %.2f ms\n", total_ns / 1e6);
    printf("   Latency: %.2f ns/lookup\n", latency_per_run);
    printf("   Verification Accumulator: %u\n", accumulated_conductions);
    
    // Rule 11 Latency Guard Gate check: Must maintain sub-microsecond latency (< 1000 ns)
    if (latency_per_run < 1000.0) {
        printf("   ✓ Latency Guard Gate PASSED (< 1000 ns).\n");
    } else {
        printf("   ⚠ Latency Guard Gate FAILED (> 1000 ns).\n");
    }
    fflush(stdout);

    // -------------------------------------------------------------
    // Benchmark 2: Verlet Capacitor Discharge Integration Step
    // -------------------------------------------------------------
    volatile double charge_v = 5.0;
    volatile double prev_charge_v = 5.0;
    volatile double dt = 0.01;
    volatile double k_decay = 5.0;
    volatile double accel = 0.0;
    volatile uint32_t final_states = 0;

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int i = 0; i < BENCH_RUNS; i++) {
        accel = -k_decay * charge_v;
        double current_v = charge_v;
        double next_v = (2.0 * current_v) - prev_charge_v + (accel * dt * dt);
        if (next_v < 0.0) next_v = 0.0;

        prev_charge_v = current_v;
        charge_v = next_v;

        if (charge_v >= 0.7) {
            final_states++;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    total_ns = (double)(end_time.tv_sec - start_time.tv_sec) * 1e9 +
               (double)(end_time.tv_nsec - start_time.tv_nsec);
    latency_per_run = total_ns / BENCH_RUNS;

    printf("\n[BENCHMARK 2] Verlet Capacitor Discharge step:\n");
    printf("   Runs: %d\n", BENCH_RUNS);
    printf("   Total Time: %.2f ms\n", total_ns / 1e6);
    printf("   Latency: %.2f ns/step\n", latency_per_run);
    printf("   State Accumulator: %u\n", final_states);

    if (latency_per_run < 1000.0) {
        printf("   ✓ Latency Guard Gate PASSED (< 1000 ns).\n");
    } else {
        printf("   ⚠ Latency Guard Gate FAILED (> 1000 ns).\n");
    }
    fflush(stdout);

    free(root);
    printf("=============================================================\n");
    printf("ALU SUBSYSTEM BENCHMARK PERFORMANCE SUITE COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
