#include "lau_thunk.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include <sys/mman.h>

// Simple context structure
typedef struct {
    float input_val;
    float output_val;
    uint8_t pad[504]; // Pad to 512
} __attribute__((aligned(512))) MyContext;

void run_test() {
    printf("[TEST] Zero-Copy Context Switching (R15 Base Pointer)...\n");

    // 1. Create Proxy
    ThunkProxy *proxy = ThunkProxy_create();
    if (!proxy) { 
        fprintf(stderr, "Failed to create proxy\n"); 
        exit(1); 
    }

    // 2. Generate Relative Thunks
    // Load from [R15 + 0] -> ZMM0
    // Store ZMM0 -> [R15 + 4]
    void (*thunk_load)(void) = (void(*)(void))ThunkProxy_emit_vgpr_load_relative(proxy, 0, 0);
    void (*thunk_store)(void) = (void(*)(void))ThunkProxy_emit_vgpr_store_relative(proxy, 4, 0);
    
    mprotect(proxy->thunk_pool, proxy->pool_size, PROT_READ | PROT_EXEC);

    // 3. Create Two Contexts
    MyContext *ctx1 = (MyContext*)lau_memalign(512, sizeof(MyContext));
    MyContext *ctx2 = (MyContext*)lau_memalign(512, sizeof(MyContext));
    
    ctx1->input_val = 10.0f;
    ctx1->output_val = 0.0f;
    
    ctx2->input_val = 20.0f;
    ctx2->output_val = 0.0f;

    // 4. Switch to Context 1
    printf("[INFO] Switching to Context 1 (Input: %.1f)\n", ctx1->input_val);
    __asm__ volatile ("movq %0, %%r15" : : "r"(ctx1) : "r15");
    
    thunk_load();  // Load 10.0 into ZMM0
    
    // Modify ZMM0 (add 1.0)
    __asm__ volatile (
        "vaddps %%zmm0, %%zmm0, %%zmm0" // Double it -> 20.0
        : : : "zmm0"
    );
    
    thunk_store(); // Store 20.0 into ctx1->output

    // 5. Switch to Context 2 (ZERO COPY)
    printf("[INFO] Switching to Context 2 (Input: %.1f)\n", ctx2->input_val);
    __asm__ volatile ("movq %0, %%r15" : : "r"(ctx2) : "r15");
    
    thunk_load(); // Load 20.0 into ZMM0
    
    // Modify ZMM0 (add 1.0)
    __asm__ volatile (
        "vaddps %%zmm0, %%zmm0, %%zmm0" // Double it -> 40.0
        : : : "zmm0"
    );
    
    thunk_store(); // Store 40.0 into ctx2->output

    // 6. Verify
    printf("[CHECK] Context 1 Output: %.1f (Expected 20.0)\n", ctx1->output_val);
    printf("[CHECK] Context 2 Output: %.1f (Expected 40.0)\n", ctx2->output_val);

    if (ctx1->output_val == 20.0f && ctx2->output_val == 40.0f) {
        printf("[SUCCESS] Zero-Copy Switching Verified.\n");
    } else {
        printf("[FAILURE] Data mismatch.\n");
        exit(1);
    }

    lau_free(ctx1);
    lau_free(ctx2);
    ThunkProxy_destroy(proxy);
}

int main() {
    run_test();
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}