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

// --- Graphics Shader Thunks ---
void tsfi_graphics_shader_object_prepare_async(VulkanContext* vk, 
                                              const uint32_t* v_spv, size_t v_size,
                                              const uint32_t* f_spv, size_t f_size,
                                              VkShaderEXT* v_out, VkShaderEXT* f_out,
                                              volatile bool* pReady);

void tsfi_ui_render_thunk(VkCommandBuffer cmd, VulkanContext* vk, 
                         VkShaderEXT v_shader, VkShaderEXT f_shader,
                         void* ui_data, size_t ui_size);

#endif
