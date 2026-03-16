#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_operator.h"
#include "tsfi_fused_operator.h"
#include "tsfi_spirv_kernels.h"
#include "vulkan_init.h"

typedef struct { float intensity; float decay; } Uniforms;
extern void wave_shader_main(const WaveStream* stream, const Uniforms* u);

int main() {
    printf("=== TSFi Operator Resonance Demo ===\n");
    VulkanContext* vk = init_vulkan();
    TSFiOperator* op = tsfi_op_create("Wave_Resonator", (void*)wave_shader_main, wave_shader_spv, wave_shader_spv_size);
    size_t sizes[] = { 1*1024*1024, 16*1024*1024, 64*1024*1024, 128*1024*1024 };
    Uniforms u = { 2.0f, 0.5f };
    for (int i = 0; i < 4; i++) {
        size_t bytes = sizes[i];
        size_t atoms = bytes / 16;
        printf("\n--- Workload: %zu MB ---\n", bytes / 1024 / 1024);
        WaveStream stream;
        stream.atom_size = 16; stream.stride = 16; stream.count = atoms;
        stream.flags = 0; // Disabled verification for performance demo
        stream.data = lau_memalign(64, bytes + 512); // Add padding for vector safety
        tsfi_op_dispatch(op, vk, &stream, &u, sizeof(u));
        if (op->last_gpu_bw > 0.0 && bytes >= op->offload_threshold) {
            printf("  [DECISION] GPU Selected (%.2f GB/s)\n", op->last_gpu_bw);
        } else {
            printf("  [DECISION] CPU Thunk Selected (%.2f GB/s)\n", op->last_cpu_bw);
        }
        lau_free(stream.data);
        op->last_gpu_bw = 0;
        op->last_cpu_bw = 0;
    }
    
    printf("\n=== Fused Operator Chain Demo (2x Op) ===\n");
    TSFiFusedOp* fused = tsfi_fused_create("Dual_Resonator");
    tsfi_fused_add(fused, op);
    tsfi_fused_add(fused, op);
    
    WaveStream fstream;
    fstream.atom_size = 16; fstream.stride = 16; fstream.count = 1024 * 1024 * 4; 
    fstream.flags = 0;
    fstream.data = lau_memalign(64, 64 * 1024 * 1024 + 512);
    
    tsfi_fused_dispatch(fused, NULL, &fstream); 
    printf("  [CPU Fused] Chain Executed Successfully.\n");
    
    lau_free(fstream.data);
    tsfi_fused_destroy(fused, vk);
    tsfi_op_destroy(op, vk);
    if (vk) cleanup_vulkan(vk);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
