#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "window_inc/vulkan_struct.h"
#include "tsfi_fused_operator.h"
#include "lau_memory.h"

// Function declared in plugins/vulkan_init.c
VulkanContext *init_vulkan(void);
void cleanup_vulkan(VulkanContext *vk);
extern void* tsfi_hotload_thunk(const char* c_file, const char* symbol_name);
extern void tsfi_dl_thunks_teardown(void);
void dummy_shader_main(const WaveStream* stream, const void* u) {
    (void)stream;
    (void)u;
}

double get_time_s() {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void run_overhead_test(int iterations) {
    printf("[BENCH] Measuring Vulkan Init/Cleanup Overhead (%d iter)...\n", iterations);
    double start = get_time_s();
    for (int i = 0; i < iterations; i++) {
        VulkanContext *vk = init_vulkan();
        if (vk) {
            VkBufferCreateInfo bufferInfo = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = 1024,
                .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
            };
            VkBuffer buffer;
            if (vk->vkCreateBuffer) {
                vk->vkCreateBuffer(vk->device, &bufferInfo, NULL, &buffer);
                vk->vkDestroyBuffer(vk->device, buffer, NULL);
            }
            cleanup_vulkan(vk);
        }
    }
    double end = get_time_s();
    printf("  -> Total Time: %.4f s (%.2f ms/init)\n", end - start, (end - start) * 1000.0 / iterations);

    printf("[BENCH] Measuring JIT Thunk Hotload Overhead (%d iter)...\n", iterations);
    start = get_time_s();
    for (int i = 0; i < iterations; i++) {
        void* ptr = tsfi_hotload_thunk("thunks/tsfi_k0rn_rasterizer.c", "tsfi_font_render_k0rn_pt");
        if (!ptr) { printf("  -> Thunk Load Failed!\n"); }
    }
    end = get_time_s();
    printf("  -> Total Time: %.4f s (%.2f ms/thunk)\n", end - start, (end - start) * 1000.0 / iterations);

    printf("[BENCH] Measuring Fused Operator Dispatch Overhead (%d iter)...\n", iterations);
    TSFiOperator* op1 = tsfi_op_create("Op1", (void*)dummy_shader_main, NULL, 0);
    TSFiOperator* op2 = tsfi_op_create("Op2", (void*)dummy_shader_main, NULL, 0);
    TSFiFusedOp* fused = tsfi_fused_create("Chain_1_2");
    tsfi_fused_add(fused, op1);
    tsfi_fused_add(fused, op2);
    
    WaveStream stream = {0};
    stream.atom_size = 16; stream.stride = 16; stream.count = 1024;
    stream.data = lau_memalign(64, stream.count * 16);
    float* data = (float*)stream.data;
    for(size_t i=0; i<stream.count*4; i++) data[i] = 1.0f;

    start = get_time_s();
    for (int i = 0; i < iterations; i++) {
        tsfi_fused_dispatch(fused, NULL, &stream);
    }
    end = get_time_s();
    printf("  -> Total Time: %.4f s (%.4f ms/dispatch)\n", end - start, (end - start) * 1000.0 / iterations);

    lau_free(stream.data);
    tsfi_fused_destroy(fused, NULL);
    tsfi_op_destroy(op1, NULL);
    tsfi_op_destroy(op2, NULL);
}

int main() {
    int iterations = 2; // Keep it low for quick check
    const char* val_env = getenv("TSFI_VALIDATION");
    int val_active = (val_env && strcmp(val_env, "0") == 0) ? 0 : 1;

    printf("=== TSFi Architecture Component Overhead Benchmark ===\n");
    printf("Validation State: %s\n", val_active ? "ENABLED" : "DISABLED");
    
    run_overhead_test(iterations);
    
    printf("\n[MEMORY METRICS]\n");
    tsfi_dl_thunks_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    lau_report_memory_metrics();
    return 0;
}