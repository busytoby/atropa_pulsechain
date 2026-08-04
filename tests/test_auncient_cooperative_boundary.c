#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <openssl/sha.h>
#include <math.h>

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
    uint32_t register_space[10];
    bool parity_error;
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

// 4. Coaxial Arbitration Structure
typedef struct {
    uint32_t active_token_node;
    bool request_lines[4];
    bool collision_flag;
} coax_arbitration_t;

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
    if (wmq->req_line > 12.0 || wmq->ack_line > 12.0) {
        wmq->overcurrent_trap = true;
        wmq->tx_gate = CUTOFF_STATE;
        wmq->rx_gate = CUTOFF_STATE;
    }

    if (wmq->overcurrent_trap) {
        sched->collision_flag = true;
        sched->scheduler_write_gate = CUTOFF_STATE;
        sched->kernel_execution_gate = CUTOFF_STATE;
    }
}

// Validation 2: Timing/Latency Path
double measure_preemption_latency_ns(scheduler_state_t *sched, uint32_t target_pid, bool mutex_locked) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

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

// Validation 3: Transistor Voltage Decay Simulation
double simulate_voltage_decay(double initial_volts, double resistance, double capacitance, int cycles) {
    double volts = initial_volts;
    double dt = 1e-12; // 1ps cycles for numerical stability
    double rc = resistance * capacitance * 1e-9; // RC time constant
    for (int i = 0; i < cycles; i++) {
        volts -= (volts * dt) / rc;
    }
    return volts;
}

// Validation 4: State Register Noise Parity Audit
bool verify_register_parity(uint32_t val) {
    int count = 0;
    for (int i = 0; i < 32; i++) {
        if ((val >> i) & 1) count++;
    }
    return (count % 2) != 0; // Odd parity
}

// Validation 5: Coaxial Arbitration loop
void run_coax_arbitration(coax_arbitration_t *arb) {
    int active_requests = 0;
    int first_active = -1;
    for (int i = 0; i < 4; i++) {
        if (arb->request_lines[i]) {
            active_requests++;
            if (first_active == -1) first_active = i;
        }
    }
    if (active_requests > 1) {
        arb->collision_flag = true;
        arb->active_token_node = 0xFFFFFFFF; // Collision locks bus
    } else if (active_requests == 1) {
        arb->collision_flag = false;
        arb->active_token_node = first_active;
    } else {
        arb->collision_flag = false;
        arb->active_token_node = 0xFFFFFFFF;
    }
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
        .collision_flag = false,
        .register_space = { 0 },
        .parity_error = false
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

    // 6. Fortification 1: Transistor Voltage Decay Simulation
    printf("[TEST] Fortification: Simulating transistor voltage decay...\n");
    double final_v = simulate_voltage_decay(5.0, 1.2, 0.05, 100); // 100 cycles decay
    assert(final_v < 1.0); // Should discharge low below logic high threshold
    printf("   ✓ Decay simulation verified: 5.0V discharged to %0.3fV.\n", final_v);
    fflush(stdout);

    // 7. Fortification 2: State Register Noise Parity Audit
    printf("[TEST] Fortification: Injecting noise and verifying parity audit...\n");
    sched.register_space[0] = 5; // binary 0101 (even count of set bits = 2)
    assert(verify_register_parity(sched.register_space[0]) == false); // Parity error caught
    sched.register_space[0] = 7; // binary 0111 (odd count of set bits = 3)
    assert(verify_register_parity(sched.register_space[0]) == true); // Parity ok
    printf("   ✓ Noise injection check verified. Parity errors caught successfully.\n");
    fflush(stdout);

    // 8. Fortification 3: Multi-Node Coaxial Collision Arbitration
    printf("[TEST] Fortification: Testing 4-node coaxial collision arbitration...\n");
    coax_arbitration_t arb = {
        .active_token_node = 0xFFFFFFFF,
        .request_lines = { false, false, false, false },
        .collision_flag = false
    };

    // Single request should succeed
    arb.request_lines[2] = true;
    run_coax_arbitration(&arb);
    assert(arb.active_token_node == 2);
    assert(arb.collision_flag == false);

    // Multiple simultaneous requests should cause collision and lock bus
    arb.request_lines[1] = true;
    run_coax_arbitration(&arb);
    assert(arb.collision_flag == true);
    assert(arb.active_token_node == 0xFFFFFFFF);
    printf("   ✓ Coaxial arbitration and collision locking verified successfully.\n");
    fflush(stdout);

    free(trace_registry);
    printf("=============================================================\n");
    printf("COOPERATIVE PIPELINE VALIDATION PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
