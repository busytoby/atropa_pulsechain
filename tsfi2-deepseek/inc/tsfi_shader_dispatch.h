#ifndef TSFI_SHADER_DISPATCH_H
#define TSFI_SHADER_DISPATCH_H

#include "lau_memory.h"
#include "tsfi_wave_any.h"
#include "window_inc/vulkan_struct.h"

// Forward declaration
struct TSFiOperator;

void tsfi_shader_pipeline_create(VulkanContext* vk, struct TSFiOperator* op, size_t pc_size);
void tsfi_shader_buffer_ensure(VulkanContext* vk, struct TSFiOperator* op, size_t size);
void tsfi_shader_pipeline_execute(VulkanContext* vk, struct TSFiOperator* op, void* buffer_ptr, size_t buffer_size, void* push_constants, size_t pc_size);
void tsfi_dispatch_shader(VulkanContext* vk, const uint32_t* spv_code, size_t spv_size, void* buffer_ptr, size_t buffer_size, void* push_constants, size_t pc_size);
void tsfi_dispatch_wave_stream(VulkanContext* vk, struct TSFiOperator* op, const WaveStream* stream, void* push_constants, size_t pc_size);

#endif
