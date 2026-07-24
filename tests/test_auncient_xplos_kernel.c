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

// -------------------------------------------------------------
// XplOS Kernel Operations
// -------------------------------------------------------------

// Schedule active process using hardware-gate simulation (branchless)
void xplos_kernel_schedule(xplos_kernel_t *kernel, uint32_t target_pid) {
    kernel->active_pid = target_pid;
    
    // Conduction resolved via target PID match (NPN simulation)
    kernel->processes[0].execution_gate = (target_pid == 0) ? CONDUC_STATE : CUTOFF_STATE;
    kernel->processes[1].execution_gate = (target_pid == 1) ? CONDUC_STATE : CUTOFF_STATE;
}

// Route hardware interrupt (STANAG/DECnet) to active process (Gated by privilege level)
bool xplos_kernel_route_interrupt(xplos_kernel_t *kernel, uint32_t interrupt_tag) {
    // Interrupt routing gate conducts only if target process holds appropriate privilege tag
    uint32_t active_tag = (kernel->active_pid == 0) ? kernel->processes[0].tag : kernel->processes[1].tag;
    
    // Interrupt is authorized only if active process has TAG_KERNEL (0x01)
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

    // Initialize process table DAT
    TwoThreeNode *proc_table = create_leaf(0, "PID:0,TAG:KERNEL");

    xplos_kernel_t kernel = {
        .processes = {
            { .pid = 0, .tag = 0x01, .execution_gate = CUTOFF_STATE }, // Kernel
            { .pid = 1, .tag = 0x02, .execution_gate = CUTOFF_STATE }  // User
        },
        .active_pid = 0,
        .interrupt_routing_gate = CUTOFF_STATE,
        .process_table = proc_table
    };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, kernel.process_table->node_hash, HASH_SIZE);

    // 1. Verify scheduling toggles gates
    printf("[TEST] Scheduling Process 0 (Kernel)...\n");
    fflush(stdout);
    xplos_kernel_schedule(&kernel, 0);
    assert(kernel.processes[0].execution_gate == CONDUC_STATE);
    assert(kernel.processes[1].execution_gate == CUTOFF_STATE);
    printf("   ✓ Process 0 scheduled (execution gate conducting).\n");
    fflush(stdout);

    // 2. Verify privileged interrupt routing (Allowed on Kernel Process 0)
    printf("[TEST] Dispatching network interrupt to Process 0...\n");
    fflush(stdout);
    bool routed = xplos_kernel_route_interrupt(&kernel, 0x01);
    assert(routed == true);
    assert(kernel.interrupt_routing_gate == CONDUC_STATE);
    printf("   ✓ Interrupt routed successfully to Process 0.\n");
    fflush(stdout);

    // 3. Switch to User Process 1 and attempt interrupt routing (Should be blocked)
    printf("[TEST] Scheduling Process 1 (User)...\n");
    fflush(stdout);
    xplos_kernel_schedule(&kernel, 1);
    assert(kernel.processes[0].execution_gate == CUTOFF_STATE);
    assert(kernel.processes[1].execution_gate == CONDUC_STATE);

    printf("[TEST] Dispatching network interrupt to Process 1...\n");
    fflush(stdout);
    routed = xplos_kernel_route_interrupt(&kernel, 0x01);
    assert(routed == false);
    assert(kernel.interrupt_routing_gate == CUTOFF_STATE); // Decoupled by hardware gate
    printf("   ✓ Interrupt blocked successfully for User Process 1.\n");
    fflush(stdout);

    // 4. Update Process Table state -> Verify Merkle root hash evolution
    printf("[TEST] Storing updated state in process table...\n");
    fflush(stdout);
    bool store_ok = store_table_data(kernel.process_table, 0, "PID:0,TAG:KERNEL,STATE:BLOCKED");
    assert(store_ok == true);
    assert(memcmp(initial_hash, kernel.process_table->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Process Table state update propagated root Merkle hash.\n");
    fflush(stdout);

    free(proc_table);
    printf("=============================================================\n");
    printf("XPLOS KERNEL INTEGRATION TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
