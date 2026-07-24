#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define ACC_MAX_LIMIT 100

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

// Extended XplOS VM state
typedef struct {
    uint32_t PC;
    uint64_t ACC;
    uint8_t tag;
    uint32_t breakpoint;
    gate_state_t write_gate;
} xplos_vm_t;

// Extended Debug Profiler
typedef struct {
    uint32_t steps_executed;
    uint32_t break_hits;
    uint32_t overflow_events;
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
// Extended Debug step and profiling
// -------------------------------------------------------------
bool xplos_debug_step_extended(xplos_vm_t *vm, xplos_profiler_t *prof) {
    // 1. Conditional Breakpoint Check: PC match
    if (vm->PC == vm->breakpoint) {
        vm->write_gate = CUTOFF_STATE;
        prof->break_hits++;
        return false;
    }

    // 2. Hardware-Gated Voltage/ACC Overcurrent Check (ACC exceeding limit)
    if (vm->ACC + 40 > ACC_MAX_LIMIT) {
        vm->write_gate = CUTOFF_STATE;
        prof->overflow_events++;
        return false; // Halted due to conditional ACC protection limit
    }

    // 3. Normal step execution
    vm->write_gate = CONDUC_STATE;
    vm->ACC += 40; // Increment accumulator
    vm->PC++;
    prof->steps_executed++;

    // 4. Log trace
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
    printf("AUNCIENT EXTENDED XPLOS DEBUGGER VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *trace_leaf = create_leaf(1, "INIT_TRACE");
    xplos_profiler_t prof = {
        .steps_executed = 0,
        .break_hits = 0,
        .overflow_events = 0,
        .trace_registry = trace_leaf
    };

    xplos_vm_t vm = {
        .PC = 2000,
        .ACC = 10,
        .tag = 0x01,
        .breakpoint = 9999, // Set high to test ACC overflow first
        .write_gate = CONDUC_STATE
    };

    // 1. Step 1 (ACC = 10 -> 50)
    printf("[TEST] Running instruction step 1 (ACC = 10)...\n");
    fflush(stdout);
    bool ok = xplos_debug_step_extended(&vm, &prof);
    assert(ok == true);
    assert(vm.PC == 2001);
    assert(vm.ACC == 50);
    assert(prof.steps_executed == 1);
    printf("   ✓ Step 1 executed successfully.\n");
    fflush(stdout);

    // 2. Step 2 (ACC = 50 -> 90)
    printf("[TEST] Running instruction step 2 (ACC = 50)...\n");
    fflush(stdout);
    ok = xplos_debug_step_extended(&vm, &prof);
    assert(ok == true);
    assert(vm.PC == 2002);
    assert(vm.ACC == 90);
    assert(prof.steps_executed == 2);
    printf("   ✓ Step 2 executed successfully.\n");
    fflush(stdout);

    // 3. Step 3 (ACC = 90 -> ACC exceeds max limit) -> Should trigger overflow cutoff
    printf("[TEST] Running instruction step 3 (ACC = 90, threshold limit = 100)...\n");
    fflush(stdout);
    ok = xplos_debug_step_extended(&vm, &prof);
    assert(ok == false); // Blocked
    assert(vm.write_gate == CUTOFF_STATE); // Cutoff active
    assert(vm.PC == 2002); // Halted
    assert(vm.ACC == 90);  // Unaltered
    assert(prof.overflow_events == 1);
    printf("   ✓ Gated overcurrent protection active: execution halted.\n");
    fflush(stdout);

    free(trace_leaf);
    printf("=============================================================\n");
    printf("EXTENDED XPLOS DEBUGGER TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
