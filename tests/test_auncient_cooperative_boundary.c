#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <openssl/sha.h>

#define HASH_SIZE 32

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

// 1. WinchesterMQ State
typedef struct {
    double req_line;
    double ack_line;
    gate_state_t tx_gate;
    gate_state_t rx_gate;
    bool overcurrent_trap;
} wmq_state_t;

// 2. Scheduler & Kernel State
typedef struct {
    uint32_t active_pid;
    gate_state_t scheduler_write_gate;
    gate_state_t kernel_execution_gate;
    bool collision_flag;
} scheduler_state_t;

// 3. Merkle Registry Node
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

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

static bool store_table_data(TwoThreeNode *node, uint32_t key, const char *new_value) {
    if (!node) return false;
    if (node->is_leaf) {
        if (node->keys[0] == key) {
            strcpy(node->values[0], new_value);
            uint8_t temp[256];
            int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
            sha256(temp, len, node->node_hash);
            return true;
        }
        return false;
    }
    return store_table_data(node->children[0], key, new_value);
}

// -------------------------------------------------------------
// COOPERATIVE VALIDATIONS
// -------------------------------------------------------------

// Validation 1: Handshake Overcurrent Cutoff Propagation
void validate_handshake_cutoff_propagation(wmq_state_t *wmq, scheduler_state_t *sched) {
    // WinchesterMQ Overcurrent Check
    if (wmq->req_line > 12.0 || wmq->ack_line > 12.0) {
        wmq->overcurrent_trap = true;
        wmq->tx_gate = CUTOFF_STATE;
        wmq->rx_gate = CUTOFF_STATE;
    }

    // Cascade: If WinchesterMQ is trapped, force scheduler cutoff and raise collision flag
    if (wmq->overcurrent_trap) {
        sched->collision_flag = true;
        sched->scheduler_write_gate = CUTOFF_STATE;
        sched->kernel_execution_gate = CUTOFF_STATE;
    }
}

// Validation 2: Timing/Latency Path (Sysfs Update to Kernel Preemption)
double measure_preemption_latency_ns(scheduler_state_t *sched, uint32_t target_pid, bool mutex_locked) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // If mutex lock is active, preemption is blocked
    if (mutex_locked) {
        sched->scheduler_write_gate = CUTOFF_STATE;
    } else {
        sched->scheduler_write_gate = CONDUC_STATE;
        sched->active_pid = target_pid;
        sched->kernel_execution_gate = CONDUC_STATE;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    return elapsed_ns;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT INTER-SYSTEM COOPERATIVE BOUNDARY VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    wmq_state_t wmq = {
        .req_line = 5.0,
        .ack_line = 0.0,
        .tx_gate = CONDUC_STATE,
        .rx_gate = CONDUC_STATE,
        .overcurrent_trap = false
    };

    scheduler_state_t sched = {
        .active_pid = 1,
        .scheduler_write_gate = CONDUC_STATE,
        .kernel_execution_gate = CONDUC_STATE,
        .collision_flag = false
    };

    TwoThreeNode *trace_registry = create_leaf(1, "PID=1,STATUS=RUNNING");
    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, trace_registry->node_hash, HASH_SIZE);

    // 1. Verify normal conduction state
    printf("[TEST] Running under normal voltage and conduction limits...\n");
    validate_handshake_cutoff_propagation(&wmq, &sched);
    assert(wmq.overcurrent_trap == false);
    assert(sched.scheduler_write_gate == CONDUC_STATE);
    printf("   ✓ System registers operating within normal limits.\n");
    fflush(stdout);

    // 2. Verify Overcurrent Cascade Cutoff
    printf("[TEST] Injecting overcurrent surge (>12V) into WinchesterMQ lines...\n");
    wmq.req_line = 15.0; // Trigger overcurrent
    validate_handshake_cutoff_propagation(&wmq, &sched);
    assert(wmq.overcurrent_trap == true);
    assert(wmq.tx_gate == CUTOFF_STATE);
    assert(sched.scheduler_write_gate == CUTOFF_STATE);
    assert(sched.kernel_execution_gate == CUTOFF_STATE);
    assert(sched.collision_flag == true);
    printf("   ✓ Handshake overcurrent cutoff successfully cascaded to scheduler execution gates.\n");
    fflush(stdout);

    // 3. Reset states and verify timing path latency
    wmq.req_line = 5.0;
    wmq.overcurrent_trap = false;
    sched.scheduler_write_gate = CONDUC_STATE;
    sched.kernel_execution_gate = CONDUC_STATE;

    printf("[TEST] Measuring preemption latency path (VFS to Kernel dispatch)...\n");
    double latency = measure_preemption_latency_ns(&sched, 2, false);
    assert(sched.active_pid == 2);
    assert(sched.kernel_execution_gate == CONDUC_STATE);
    printf("   ✓ Preemption dispatched in %0.2fns.\n", latency);
    fflush(stdout);

    // 4. Verify Mutex Serialization preemption blocking
    printf("[TEST] Testing Mutex locked preemption path...\n");
    latency = measure_preemption_latency_ns(&sched, 3, true); // Attempt preemption with mutex active
    assert(sched.scheduler_write_gate == CUTOFF_STATE); // Waking blocked
    printf("   ✓ Mutex preemption block verified successfully.\n");
    fflush(stdout);

    // 5. Verify Cryptographic Merkle State Audit Trail
    printf("[TEST] Writing dispatch event to Merkle Trace Registry...\n");
    store_table_data(trace_registry, 1, "PID=2,STATUS=DISPATCHED");
    assert(memcmp(initial_hash, trace_registry->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Transition cryptographically validated and logged to trace tree.\n");
    fflush(stdout);

    free(trace_registry);
    printf("=============================================================\n");
    printf("COOPERATIVE PIPELINE VALIDATION PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
