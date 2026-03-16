#ifndef TSFI_OPERATOR_H
#define TSFI_OPERATOR_H

#include "lau_memory.h"
#include "lau_thunk.h"
#include "tsfi_wave_any.h"
#include "window_inc/vulkan_struct.h"

typedef struct TSFiOperator {
    // --- Bijective Identity ---
    const char* name;
    uint64_t signature; // CRC32/Hash of logic
    
    // --- Execution Paths ---
    ThunkProxy* cpu_thunk;      // JIT-compiled AVX-512 path
    void (*cpu_entry)(void);    // Direct function pointer to thunk
    
    // Cached GPU Resources (The "GPU Thunk")
    VkPipeline gpu_pipeline;    
    VkPipelineLayout gpu_layout;
    VkDescriptorSetLayout gpu_descriptor_layout;
    VkDescriptorPool gpu_pool;
    VkDescriptorSet gpu_set;
    VkShaderModule gpu_module;
    VkCommandBuffer gpu_cmd; // Cached/Baked Command Buffer
    size_t baked_size;       // Size used for the current bake
    
    // Shared I/O Buffer (Optional: For persistent mapping)
    VkBuffer gpu_buffer;
    VkDeviceMemory gpu_memory;
    void* gpu_mapped_ptr;
    
    // Crossover/Verify Buffers
    void* verify_backup;
    void* verify_result;
    size_t verify_capacity;
    
    const uint32_t* spv_code;   // SPIR-V Blob
    size_t spv_size;
    
    // --- Resonance State ---
    double last_cpu_bw;         // GB/s
    double last_gpu_bw;         // GB/s
    uint64_t execution_count;
    
    // --- Thresholds ---
    size_t offload_threshold;   // Bytes (Dynamic)
} TSFiOperator;

TSFiOperator* tsfi_op_create(const char* name, void* cpu_fn, const uint32_t* spv, size_t spv_size);
void tsfi_op_dispatch(TSFiOperator* op, VulkanContext* vk, WaveStream* stream, void* uniforms, size_t u_size);
void tsfi_op_destroy(TSFiOperator* op, VulkanContext* vk);

#endif
