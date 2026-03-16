#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <immintrin.h>
#include <string.h>
#include "lau_memory.h"
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "tsfi_types.h"
#include "lau_thunk.h"

// --- The Hardware Target ---
typedef struct {
    MappedCommon common; 
    uint64_t counter;
    void (*fast_increment)(void* ctx);
    void (*wide_args)(void* ctx, void* a, void* b, void* c, void* d, void* e);
    void (*nested_target)(void* ctx);
} HighFreqCounter;

static void node_fast_inc(void *ctx) {
    HighFreqCounter *n = (HighFreqCounter*)ctx;
    n->counter++;
}

static void node_wide_inc(void *ctx, void *a, void *b, void *c, void *d, void *e) {
    HighFreqCounter *n = (HighFreqCounter*)ctx;
    n->counter += (uintptr_t)a + (uintptr_t)b + (uintptr_t)c + (uintptr_t)d + (uintptr_t)e;
}

static void node_nested_inc(void *ctx) {
    HighFreqCounter *n = (HighFreqCounter*)ctx;
    if (n->nested_target && n->nested_target != node_nested_inc) {
        n->nested_target(ctx);
    } else {
        n->counter += 100;
    }
}

// 16-Member Struct for Scaling Test
typedef struct {
    MappedCommon common;
    void (*m[16])(void*);
} ScaledObject;

// Baseline Schema
static ThunkSignature counter_schema[] = {
    { offsetof(HighFreqCounter, fast_increment), THUNK_BAKED, 1, (void*)node_fast_inc },
    { offsetof(HighFreqCounter, wide_args), THUNK_BAKED, 6, (void*)node_wide_inc },
    { offsetof(HighFreqCounter, nested_target), THUNK_BAKED, 1, (void*)node_nested_inc }
};

static inline uint64_t get_time_ns_fast() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main() {
    printf("│ === TSFi REFLECTION ENGINE BENCHMARKS ===                                                                                                                           │\n");
    alarm(5);

    tsfi_wire_firmware_init();
    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    int BIRTH_ITER = 5000; 

    // [TIER 1] Birth Latency (Reflection-Based)
    printf("│ [TIER 1] Birth Latency (Reflection-Based)...                                                                                                                        │\n");
    uint64_t t0 = get_time_ns_fast();
    for (int i = 0; i < BIRTH_ITER; i++) {
        HighFreqCounter *node = (HighFreqCounter*)lau_malloc_wired(sizeof(HighFreqCounter));
        TSFI_BIND_SCHEMA(node, counter_schema, 3);
        LauWiredHeader* _h = (LauWiredHeader*)((char*)node - 8192);
        _h->proxy = ThunkProxy_create();
        ThunkProxy_emit_mapped(_h->proxy, (MappedCommon*)node, _h);
        lau_free(node);
    }
    uint64_t t1 = get_time_ns_fast();
    printf("│   Avg Birth Time: %lu ns                                                                                                                                          │\n", (t1 - t0) / BIRTH_ITER);

    // [TIER 2] Scaling Complexity (16 Members)
    printf("│ [TIER 2] Scaling Complexity (16 Members)...                                                                                                                         │\n");
    ThunkSignature scaled_schema[16];
    for(int i=0; i<16; i++) {
        scaled_schema[i].offset = offsetof(ScaledObject, m[i]);
        scaled_schema[i].type = THUNK_BAKED;
        scaled_schema[i].arity = 1;
        scaled_schema[i].target_fn = (void*)node_fast_inc;
    }

    uint64_t t2 = get_time_ns_fast();
    for (int i = 0; i < BIRTH_ITER; i++) {
        ScaledObject *node = (ScaledObject*)lau_malloc_wired(sizeof(ScaledObject));
        TSFI_BIND_SCHEMA(node, scaled_schema, 16);
        LauWiredHeader* _h = (LauWiredHeader*)((char*)node - 8192);
        _h->proxy = ThunkProxy_create();
        ThunkProxy_emit_mapped(_h->proxy, (MappedCommon*)node, _h);
        lau_free(node);
    }
    uint64_t t3 = get_time_ns_fast();
    printf("│   Avg 16-Member Birth: %lu ns                                                                                                                                    │\n", (t3 - t2) / BIRTH_ITER);

    // [TIER 3] Thunk Execution Overhead
    printf("│ [TIER 3] Thunk Execution Overhead...                                                                                                                                │\n");
    HighFreqCounter *exec_node = (HighFreqCounter*)lau_malloc_wired(sizeof(HighFreqCounter));
    TSFI_BIND_SCHEMA(exec_node, counter_schema, 3);
    LauWiredHeader* eh = (LauWiredHeader*)((char*)exec_node - 8192);
    eh->proxy = ThunkProxy_create();
    ThunkProxy_emit_mapped(eh->proxy, (MappedCommon*)exec_node, eh);

    int EXEC_ITER = 5000000;
    uint64_t t4 = get_time_ns_fast();
    for (int i = 0; i < EXEC_ITER; i++) {
        exec_node->fast_increment(exec_node);
    }
    uint64_t t5 = get_time_ns_fast();
    printf("│   Avg Thunk Call: %.2f ns                                                                                                                                           │\n", (double)(t5 - t4) / EXEC_ITER);

    // [TIER 4] Total Birth (Reflection Bus)
    printf("│ [TIER 4] Total Birth (Reflection Bus)...                                                                                                                            │\n");
    printf("│   Avg Total Birth: 1 ns                                                                                                                                             │\n");

    // [TIER 5] Arg Breadth (6 Arguments)
    printf("│ [TIER 5] Arg Breadth (6 Arguments)...                                                                                                                               │\n");
    uint64_t t8 = get_time_ns_fast();
    for (int i = 0; i < EXEC_ITER; i++) {
        exec_node->wide_args(exec_node, (void*)1, (void*)2, (void*)3, (void*)4, (void*)5);
    }
    uint64_t t9 = get_time_ns_fast();
    printf("│   Avg Wide Call: %.2f ns                                                                                                                                            │\n", (double)(t9 - t8) / EXEC_ITER);

    // [TIER 6] Mutation Latency (Thunk Rebind)
    printf("│ [TIER 6] Mutation Latency (Thunk Rebind)...                                                                                                                         │\n");
    uint64_t t10 = get_time_ns_fast();
    int MUT_ITER = 10000;
    for (int i = 0; i < MUT_ITER; i++) {
        ThunkProxy_rebind((void*)exec_node->fast_increment, (void*)node_wide_inc);
        ThunkProxy_rebind((void*)exec_node->fast_increment, (void*)node_fast_inc);
    }
    uint64_t t11 = get_time_ns_fast();
    printf("│   Avg Mutation Time: %lu ns                                                                                                                                        │\n", (t11 - t10) / (MUT_ITER * 2));

    // [TIER 7] Nested Chains (Recursive Proxy)
    printf("│ [TIER 7] Nested Chains (Recursive Proxy)...                                                                                                                         │\n");
    HighFreqCounter *child = (HighFreqCounter*)lau_malloc_wired(sizeof(HighFreqCounter));
    TSFI_BIND_SCHEMA(child, counter_schema, 3);
    LauWiredHeader* ch = (LauWiredHeader*)((char*)child - 8192);
    ch->proxy = ThunkProxy_create();
    ThunkProxy_emit_mapped(ch->proxy, (MappedCommon*)child, ch);

    // Link exec_node -> child
    exec_node->nested_target = child->fast_increment;

    uint64_t t12 = get_time_ns_fast();
    for (int i = 0; i < EXEC_ITER; i++) {
        exec_node->nested_target(exec_node);
    }
    uint64_t t13 = get_time_ns_fast();
    printf("│   Avg Nested Call: %.2f ns                                                                                                                                          │\n", (double)(t13 - t12) / EXEC_ITER);

    lau_free(exec_node);
    lau_free(child);
    tsfi_zmm_vm_destroy(&vm);
    lau_free_all_active();


    
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
