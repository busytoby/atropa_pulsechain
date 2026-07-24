#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define CLUSTER_SIZE 3

// Simulated CSCW PN-Counter CRDT
typedef struct {
    uint32_t P[CLUSTER_SIZE]; // Positive increments array
    uint32_t N[CLUSTER_SIZE]; // Negative decrements array
} pn_counter_t;

// Helper to return the max of two values
static inline uint32_t max_val(uint32_t a, uint32_t b) {
    return (a > b) ? a : b;
}

// -------------------------------------------------------------
// PN-Counter Operations
// -------------------------------------------------------------
void increment_pn_counter(pn_counter_t *counter, uint32_t node_id, uint32_t val) {
    if (node_id < CLUSTER_SIZE) {
        counter->P[node_id] += val;
    }
}

void decrement_pn_counter(pn_counter_t *counter, uint32_t node_id, uint32_t val) {
    if (node_id < CLUSTER_SIZE) {
        counter->N[node_id] += val;
    }
}

int32_t resolve_pn_counter_value(const pn_counter_t *counter) {
    int32_t sum_p = 0;
    int32_t sum_n = 0;
    for (int i = 0; i < CLUSTER_SIZE; i++) {
        sum_p += (int32_t)counter->P[i];
        sum_n += (int32_t)counter->N[i];
    }
    return sum_p - sum_n;
}

// -------------------------------------------------------------
// CRDT Merge Rule: Element-wise Maximum
// -------------------------------------------------------------
void merge_pn_counter(pn_counter_t *local, const pn_counter_t *incoming) {
    for (int i = 0; i < CLUSTER_SIZE; i++) {
        local->P[i] = max_val(local->P[i], incoming->P[i]);
        local->N[i] = max_val(local->N[i], incoming->N[i]);
    }
}

// -------------------------------------------------------------
// Synthesizer Accumulator Integration
// -------------------------------------------------------------
uint32_t resolve_synth_accumulator_phase(const pn_counter_t *counter, uint32_t phase_limit) {
    int32_t net_value = resolve_pn_counter_value(counter);
    if (net_value < 0) {
        // Handle negative wrapping
        int32_t remainder = net_value % (int32_t)phase_limit;
        if (remainder < 0) {
            remainder += (int32_t)phase_limit;
        }
        return (uint32_t)remainder;
    }
    return (uint32_t)net_value % phase_limit;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT CSCW PN-COUNTER CRDT SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    pn_counter_t node0 = { {0}, {0} };
    pn_counter_t node1 = { {0}, {0} };

    // 1. Perform local operations on Node 0 and Node 1
    increment_pn_counter(&node0, 0, 10); // Node 0 increment +10
    decrement_pn_counter(&node0, 0, 3);  // Node 0 decrement -3

    increment_pn_counter(&node1, 1, 15); // Node 1 increment +15
    decrement_pn_counter(&node1, 1, 5);  // Node 1 decrement -5

    assert(resolve_pn_counter_value(&node0) == 7);
    assert(resolve_pn_counter_value(&node1) == 10);
    printf("[TEST] Local increments and decrements verified.\n");
    fflush(stdout);

    // 2. Perform merge synchronization -> Node 0 merges updates from Node 1
    printf("[TEST] Merging PN-counter states across nodes...\n");
    fflush(stdout);
    merge_pn_counter(&node0, &node1);
    
    // Expected value: (10 + 15) - (3 + 5) = 25 - 8 = 17
    int32_t resolved = resolve_pn_counter_value(&node0);
    assert(resolved == 17);
    printf("   ✓ Eventual consistency reached. Resolved value: %d\n", resolved);
    fflush(stdout);

    // 3. Synthesizer Accumulator Integration Case -> Should wrap phase correctly
    printf("[TEST] Checking PN-counter as synthesizer phase accumulator (positive)...\n");
    fflush(stdout);
    uint32_t phase = resolve_synth_accumulator_phase(&node0, 1024);
    assert(phase == 17); // 17 % 1024 = 17
    printf("   ✓ Positive phase accumulator resolved to %u.\n", phase);
    fflush(stdout);

    printf("[TEST] Checking PN-counter as synthesizer phase accumulator (negative)...\n");
    fflush(stdout);
    decrement_pn_counter(&node0, 0, 30); // Net value becomes 17 - 30 = -13
    phase = resolve_synth_accumulator_phase(&node0, 1024);
    assert(phase == 1011); // -13 % 1024 wraps to 1011
    printf("   ✓ Wrapped negative phase accumulator resolved to %u.\n", phase);
    fflush(stdout);

    printf("=============================================================\n");
    printf("CSCW PN-COUNTER TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
