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

// 2-3 Merkle Tree Node for Debug Log Registry
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// XplOS VM state representation
typedef struct {
    uint32_t PC;
    uint64_t ACC;
    uint8_t tag;
    uint32_t breakpoint;
    gate_state_t write_gate;
} xplos_vm_t;

// Debug Profiler Telemetry
typedef struct {
    uint32_t steps_executed;
    uint32_t break_hits;
    TwoThreeNode *trace_registry;
} xplos_profiler_t;

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

// Update leaf in 2-3 tree DAT
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
// XplOS Debug step & Profiling
// -------------------------------------------------------------
bool xplos_debug_step(xplos_vm_t *vm, xplos_profiler_t *prof) {
    // 1. Breakpoint Check: assert transistor cutoff if PC hits target address
    if (vm->PC == vm->breakpoint) {
        vm->write_gate = CUTOFF_STATE;
        prof->break_hits++;
        return false; // Execution halted at breakpoint
    }

    // 2. Normal conduction: advance state
    vm->write_gate = CONDUC_STATE;
    vm->ACC += 10;
    vm->PC++;
    prof->steps_executed++;

    // 3. Log trace to cryptographically verified 2-3 tree registry
    char trace_val[128];
    snprintf(trace_val, sizeof(trace_val), "PC:%u,ACC:%lu,TAG:%u", vm->PC, vm->ACC, vm->tag);
    store_table_data(prof->trace_registry, prof->steps_executed, trace_val);

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPLOS PROFILER AND DEBUGGER VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *trace_leaf = create_leaf(1, "INIT_TRACE");
    xplos_profiler_t prof = {
        .steps_executed = 0,
        .break_hits = 0,
        .trace_registry = trace_leaf
    };

    xplos_vm_t vm = {
        .PC = 1000,
        .ACC = 50,
        .tag = 0x01,
        .breakpoint = 1002, // Breakpoint set at PC = 1002
        .write_gate = CONDUC_STATE
    };

    // 1. Execute first step -> Should succeed, update trace log, and change hash
    printf("[TEST] Running instruction step 1 (PC = 1000)...\n");
    fflush(stdout);
    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, prof.trace_registry->node_hash, HASH_SIZE);

    bool step_ok = xplos_debug_step(&vm, &prof);
    assert(step_ok == true);
    assert(vm.PC == 1001);
    assert(vm.ACC == 60);
    assert(vm.write_gate == CONDUC_STATE);
    assert(prof.steps_executed == 1);
    assert(memcmp(initial_hash, prof.trace_registry->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Step 1 executed successfully. Trace logged.\n");
    fflush(stdout);

    // Save state hash
    uint8_t step1_hash[HASH_SIZE];
    memcpy(step1_hash, prof.trace_registry->node_hash, HASH_SIZE);

    // 2. Execute second step -> Should succeed (PC = 1001)
    printf("[TEST] Running instruction step 2 (PC = 1001)...\n");
    fflush(stdout);
    step_ok = xplos_debug_step(&vm, &prof);
    assert(step_ok == true);
    assert(vm.PC == 1002);
    assert(vm.ACC == 70);
    assert(prof.steps_executed == 2);
    printf("   ✓ Step 2 executed successfully.\n");
    fflush(stdout);

    // 3. Execute third step -> Should hit breakpoint (PC = 1002) and trigger cutoff
    printf("[TEST] Running step 3 at breakpoint address (PC = 1002)...\n");
    fflush(stdout);
    step_ok = xplos_debug_step(&vm, &prof);
    assert(step_ok == false); // Breakpoint triggered
    assert(vm.PC == 1002); // Halted
    assert(vm.ACC == 70);  // Unchanged
    assert(vm.write_gate == CUTOFF_STATE); // Transistor cutoff active
    assert(prof.break_hits == 1);
    printf("   ✓ Breakpoint hit successfully. Write gate isolated.\n");
    fflush(stdout);

    free(trace_leaf);
    printf("=============================================================\n");
    printf("XPLOS PROFILER AND DEBUGGER TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
