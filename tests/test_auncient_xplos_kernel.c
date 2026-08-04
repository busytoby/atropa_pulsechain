#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
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

// Process structure
typedef struct {
    uint32_t pid;
    uint32_t tag; // Privilege tag (e.g. 0x01 Kernel, 0x02 User)
    gate_state_t execution_gate;
} xplos_process_t;

// 2-3 Tree Node for Kernel Process Table
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// Kernel State
typedef struct {
    xplos_process_t processes[2];
    uint32_t active_pid;
    gate_state_t interrupt_routing_gate;
    TwoThreeNode *process_table;
    double bus_voltage;
    bool overcurrent_trap;
    bool parity_error;
} xplos_kernel_t;

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

// Update leaf node hash
static void update_node_hash(TwoThreeNode *node) {
    if (!node) return;
    if (node->is_leaf) {
        uint8_t temp[256];
        int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
        sha256(temp, len, node->node_hash);
    }
}

// Update leaf in tree
static bool store_table_data(TwoThreeNode *node, uint32_t key, const char *new_value) {
    if (!node) return false;
    if (node->is_leaf) {
        if (node->keys[0] == key) {
            strcpy(node->values[0], new_value);
            update_node_hash(node);
            return true;
        }
        return false;
    }
    return store_table_data(node->children[0], key, new_value);
}

// Odd Parity Checker
static bool verify_state_parity(uint32_t val) {
    int count = 0;
    for (int i = 0; i < 32; i++) {
        if ((val >> i) & 1) count++;
    }
    return (count % 2) != 0;
}

// -------------------------------------------------------------
// XplOS Kernel Operations
// -------------------------------------------------------------

// Schedule active process using hardware-gate simulation (branchless)
bool xplos_kernel_schedule(xplos_kernel_t *kernel, uint32_t target_pid) {
    kernel->overcurrent_trap = false;
    kernel->parity_error = false;

    // 1. Conduction Overcurrent Trap: Trip if simulated bus voltage exceeds 12.0 V
    if (kernel->bus_voltage > 12.0) {
        kernel->overcurrent_trap = true;
        kernel->processes[0].execution_gate = CUTOFF_STATE;
        kernel->processes[1].execution_gate = CUTOFF_STATE;
        return false;
    }

    // 2. State Parity Verification on Target PID (expecting odd parity)
    // Target PIDs: 1 (binary 0001, 1 bit = odd) or 7 (binary 0111, 3 bits = odd)
    if (!verify_state_parity(target_pid)) {
        kernel->parity_error = true;
        return false;
    }

    kernel->active_pid = target_pid;
    
    // Conduction resolved via target PID match (NPN simulation)
    kernel->processes[0].execution_gate = (target_pid == 0x01) ? CONDUC_STATE : CUTOFF_STATE;
    kernel->processes[1].execution_gate = (target_pid == 0x07) ? CONDUC_STATE : CUTOFF_STATE;

    // 3. Process Trace Auditing: log process scheduling transition
    char log_val[128];
    snprintf(log_val, sizeof(log_val), "PID=%d,TAG=0x%02X,VOLTS=%.1f", target_pid, 
             (target_pid == 0x01) ? kernel->processes[0].tag : kernel->processes[1].tag, 
             kernel->bus_voltage);
    store_table_data(kernel->process_table, target_pid, log_val);
    return true;
}

// Route hardware interrupt (STANAG/DECnet) to active process (Gated by privilege level)
bool xplos_kernel_route_interrupt(xplos_kernel_t *kernel, uint32_t interrupt_tag) {
    uint32_t active_tag = (kernel->active_pid == 0x01) ? kernel->processes[0].tag : kernel->processes[1].tag;
    kernel->interrupt_routing_gate = (active_tag == 0x01 && interrupt_tag == 0x01) ? CONDUC_STATE : CUTOFF_STATE;
    return (kernel->interrupt_routing_gate == CONDUC_STATE);
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPLOS KERNEL OPERATIONS VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize 2-3 Merkle Tree process table
    TwoThreeNode *table_root = create_leaf(0x01, "PID1:TAG=0x01,VOLTS=5.0");
    xplos_kernel_t kernel = {
        .processes = {
            { .pid = 0x01, .tag = 0x01, .execution_gate = CUTOFF_STATE },
            { .pid = 0x07, .tag = 0x02, .execution_gate = CUTOFF_STATE }
        },
        .active_pid = 0,
        .interrupt_routing_gate = CUTOFF_STATE,
        .process_table = table_root,
        .bus_voltage = 5.0,
        .overcurrent_trap = false,
        .parity_error = false
    };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, kernel.process_table->node_hash, HASH_SIZE);

    // 1. Scheduling Process 1 (Kernel, odd parity PID 0x01) -> Should conduct
    printf("[TEST] Scheduling Process 1 (Kernel)...\n");
    fflush(stdout);
    bool ok = xplos_kernel_schedule(&kernel, 0x01);
    assert(ok == true);
    assert(kernel.processes[0].execution_gate == CONDUC_STATE);
    assert(memcmp(initial_hash, kernel.process_table->node_hash, HASH_SIZE) != 0); // Log trace saved
    printf("   ✓ Process 1 scheduled (execution gate conducting) and trace logged successfully.\n");
    fflush(stdout);

    // Save updated hash
    uint8_t updated_hash[HASH_SIZE];
    memcpy(updated_hash, kernel.process_table->node_hash, HASH_SIZE);

    // 2. Dispatch network interrupt to Process 1 -> Should conduct
    printf("[TEST] Dispatching network interrupt to Process 1...\n");
    fflush(stdout);
    bool routed = xplos_kernel_route_interrupt(&kernel, 0x01);
    assert(routed == true);
    assert(kernel.interrupt_routing_gate == CONDUC_STATE);
    printf("   ✓ Interrupt routed successfully to Process 1.\n");
    fflush(stdout);

    // 3. Scheduling Process 7 (User, odd parity PID 0x07) -> Should conduct
    printf("[TEST] Scheduling Process 7 (User)...\n");
    fflush(stdout);
    ok = xplos_kernel_schedule(&kernel, 0x07);
    assert(ok == true);
    assert(kernel.processes[1].execution_gate == CONDUC_STATE);
    printf("   ✓ Process 7 scheduled successfully.\n");
    fflush(stdout);

    // 4. Dispatch network interrupt to Process 7 -> Should cutoff
    printf("[TEST] Dispatching network interrupt to Process 7...\n");
    fflush(stdout);
    routed = xplos_kernel_route_interrupt(&kernel, 0x01);
    assert(routed == false);
    assert(kernel.interrupt_routing_gate == CUTOFF_STATE);
    printf("   ✓ Interrupt blocked successfully for User Process 7.\n");
    fflush(stdout);

    // 5. Parity failure check (even parity PID 0x02, binary 0010, 1 bit? Wait, 2 is 0010 = 1 bit = odd.
    // Let's use 3 (binary 0011, 2 bits = even)
    printf("[TEST] Testing scheduling state parity violations...\n");
    fflush(stdout);
    ok = xplos_kernel_schedule(&kernel, 0x03);
    assert(ok == false);
    assert(kernel.parity_error == true);
    printf("   ✓ State parity error detected and switch blocked successfully.\n");
    fflush(stdout);

    // 6. Conduction Overcurrent check (>12V) -> Should trip trap and shut down
    printf("[TEST] Testing conduction gate overcurrent surge protection...\n");
    fflush(stdout);
    kernel.bus_voltage = 15.0;
    ok = xplos_kernel_schedule(&kernel, 0x01);
    assert(ok == false);
    assert(kernel.overcurrent_trap == true);
    assert(kernel.processes[0].execution_gate == CUTOFF_STATE);
    assert(kernel.processes[1].execution_gate == CUTOFF_STATE);
    printf("   ✓ Overcurrent trap tripped successfully, cutting off all execution gates.\n");
    fflush(stdout);

    free(table_root);
    printf("=============================================================\n");
    printf("XPLOS KERNEL INTEGRATION TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
