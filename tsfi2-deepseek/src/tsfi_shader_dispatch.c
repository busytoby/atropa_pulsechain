#include "tsfi_shader_dispatch.h"
#include "tsfi_operator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const VkAllocationCallbacks tsfi_alloc_callbacks;

void tsfi_shader_pipeline_create(VulkanContext* vk, struct TSFiOperator* op, size_t pc_size) {
    if (!vk || !op || !op->spv_code) return;
    VkShaderModuleCreateInfo moduleInfo = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .codeSize = op->spv_size, .pCode = op->spv_code };
    if (vk->vkCreateShaderModule(vk->device, &moduleInfo, &tsfi_alloc_callbacks, &op->gpu_module) != VK_SUCCESS) return;
    VkDescriptorSetLayoutBinding binding = { .binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT };
    VkDescriptorSetLayoutCreateInfo layoutInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .bindingCount = 1, .pBindings = &binding };
    if (vk->vkCreateDescriptorSetLayout(vk->device, &layoutInfo, &tsfi_alloc_callbacks, &op->gpu_descriptor_layout) != VK_SUCCESS) return;
    VkPushConstantRange pcRange = { .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT, .offset = 0, .size = (uint32_t)pc_size };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .setLayoutCount = 1, .pSetLayouts = &op->gpu_descriptor_layout, .pushConstantRangeCount = pc_size > 0 ? 1 : 0, .pPushConstantRanges = pc_size > 0 ? &pcRange : NULL };
    if (vk->vkCreatePipelineLayout(vk->device, &pipelineLayoutInfo, &tsfi_alloc_callbacks, &op->gpu_layout) != VK_SUCCESS) return;
    VkComputePipelineCreateInfo pipelineInfo = { .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, .stage = { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_COMPUTE_BIT, .module = op->gpu_module, .pName = "main" }, .layout = op->gpu_layout };
    if (vk->vkCreateComputePipelines(vk->device, VK_NULL_HANDLE, 1, &pipelineInfo, &tsfi_alloc_callbacks, &op->gpu_pipeline) != VK_SUCCESS) return;
    VkDescriptorPoolSize poolSize = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 };
    VkDescriptorPoolCreateInfo poolInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, .poolSizeCount = 1, .pPoolSizes = &poolSize, .maxSets = 1 };
    if (vk->vkCreateDescriptorPool(vk->device, &poolInfo, &tsfi_alloc_callbacks, &op->gpu_pool) != VK_SUCCESS) return;
    VkDescriptorSetAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, .descriptorPool = op->gpu_pool, .descriptorSetCount = 1, .pSetLayouts = &op->gpu_descriptor_layout };
    if (vk->vkAllocateDescriptorSets(vk->device, &allocInfo, &op->gpu_set) != VK_SUCCESS) return;
}

void tsfi_shader_buffer_ensure(VulkanContext* vk, struct TSFiOperator* op, size_t size) {
    if (!vk || !op) return;
    if (op->gpu_buffer) {
        vk->vkDestroyBuffer(vk->device, op->gpu_buffer, &tsfi_alloc_callbacks);
        vk->vkFreeMemory(vk->device, op->gpu_memory, &tsfi_alloc_callbacks);
        op->gpu_buffer = VK_NULL_HANDLE;
    }
    VkBufferCreateInfo bufInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size, .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, .sharingMode = VK_SHARING_MODE_EXCLUSIVE };
    vk->vkCreateBuffer(vk->device, &bufInfo, &tsfi_alloc_callbacks, &op->gpu_buffer);
    VkMemoryRequirements memReqs;
    vk->vkGetBufferMemoryRequirements(vk->device, op->gpu_buffer, &memReqs);
    VkMemoryAllocateInfo memAlloc = { .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, .allocationSize = memReqs.size, .memoryTypeIndex = 0 };
    for (uint32_t i=0; i<vk->memory_properties.memoryTypeCount; i++) { if ((memReqs.memoryTypeBits & (1 << i)) && (vk->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) { memAlloc.memoryTypeIndex = i; break; } }
    vk->vkAllocateMemory(vk->device, &memAlloc, &tsfi_alloc_callbacks, &op->gpu_memory);
    vk->vkBindBufferMemory(vk->device, op->gpu_buffer, op->gpu_memory, 0);
    vk->vkMapMemory(vk->device, op->gpu_memory, 0, size, 0, &op->gpu_mapped_ptr);
    VkDescriptorBufferInfo bufferDescriptor = { .buffer = op->gpu_buffer, .offset = 0, .range = size };
    VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = op->gpu_set, .dstBinding = 0, .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .pBufferInfo = &bufferDescriptor };
    vk->vkUpdateDescriptorSets(vk->device, 1, &write, 0, NULL);
    if (op->gpu_cmd) { vk->vkFreeCommandBuffers(vk->device, vk->command_pool, 1, &op->gpu_cmd); op->gpu_cmd = VK_NULL_HANDLE; }
}

static void tsfi_shader_bake(VulkanContext* vk, struct TSFiOperator* op, size_t buffer_size, void* push_constants, size_t pc_size) {
    VkCommandBufferAllocateInfo cmdAlloc = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool = vk->command_pool, .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, .commandBufferCount = 1 };
    vk->vkAllocateCommandBuffers(vk->device, &cmdAlloc, &op->gpu_cmd);
    VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vk->vkBeginCommandBuffer(op->gpu_cmd, &beginInfo);
    vk->vkCmdBindPipeline(op->gpu_cmd, VK_PIPELINE_BIND_POINT_COMPUTE, op->gpu_pipeline);
    vk->vkCmdBindDescriptorSets(op->gpu_cmd, VK_PIPELINE_BIND_POINT_COMPUTE, op->gpu_layout, 0, 1, &op->gpu_set, 0, NULL);
    if (pc_size > 0 && vk->vkCmdPushConstants) vk->vkCmdPushConstants(op->gpu_cmd, op->gpu_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, (uint32_t)pc_size, push_constants);
    vk->vkCmdDispatch(op->gpu_cmd, (uint32_t)(buffer_size / 64) + 1, 1, 1);
    vk->vkEndCommandBuffer(op->gpu_cmd);
}

void tsfi_shader_pipeline_execute(VulkanContext* vk, struct TSFiOperator* op, void* buffer_ptr, size_t buffer_size, void* push_constants, size_t pc_size) {
    if (!vk || !op || !op->gpu_pipeline) return;
    LauGpuHandle h = lau_gpu_lookup(buffer_ptr);
    if (h.valid) {
        VkDescriptorBufferInfo bufferDescriptor = { .buffer = (VkBuffer)h.buffer, .offset = h.secret, .range = buffer_size };
        VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = op->gpu_set, .dstBinding = 0, .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .pBufferInfo = &bufferDescriptor };
        vk->vkUpdateDescriptorSets(vk->device, 1, &write, 0, NULL);
    } else {
        if (!op->gpu_buffer) tsfi_shader_buffer_ensure(vk, op, buffer_size);
        if (op->gpu_mapped_ptr && buffer_ptr != op->gpu_mapped_ptr) memcpy(op->gpu_mapped_ptr, buffer_ptr, buffer_size);
    }
    if (!op->gpu_cmd || op->baked_size != buffer_size) {
        if (op->gpu_cmd) vk->vkFreeCommandBuffers(vk->device, vk->command_pool, 1, &op->gpu_cmd);
        tsfi_shader_bake(vk, op, buffer_size, push_constants, pc_size);
        op->baked_size = buffer_size;
    }
    VkSubmitInfo submitInfo = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &op->gpu_cmd };
    vk->vkQueueSubmit(vk->queue, 1, &submitInfo, VK_NULL_HANDLE);
    vk->vkQueueWaitIdle(vk->queue);
}

void tsfi_dispatch_shader(VulkanContext* vk, const uint32_t* spv_code, size_t spv_size, void* buffer_ptr, size_t buffer_size, void* push_constants, size_t pc_size) {
    TSFiOperator op = { .spv_code = spv_code, .spv_size = spv_size };
    tsfi_shader_pipeline_create(vk, &op, pc_size);
    tsfi_shader_pipeline_execute(vk, &op, buffer_ptr, buffer_size, push_constants, pc_size);
    if (op.gpu_buffer) { vk->vkDestroyBuffer(vk->device, op.gpu_buffer, NULL); vk->vkFreeMemory(vk->device, op.gpu_memory, NULL); }
    if (op.gpu_pipeline) vk->vkDestroyPipeline(vk->device, op.gpu_pipeline, NULL);
    if (op.gpu_layout) vk->vkDestroyPipelineLayout(vk->device, op.gpu_layout, NULL);
    if (op.gpu_descriptor_layout) vk->vkDestroyDescriptorSetLayout(vk->device, op.gpu_descriptor_layout, NULL);
    if (op.gpu_pool) vk->vkDestroyDescriptorPool(vk->device, op.gpu_pool, NULL);
    if (op.gpu_module) vk->vkDestroyShaderModule(vk->device, op.gpu_module, NULL);
    if (op.gpu_cmd) vk->vkFreeCommandBuffers(vk->device, vk->command_pool, 1, &op.gpu_cmd);
}

void tsfi_dispatch_wave_stream(VulkanContext* vk, struct TSFiOperator* op, const WaveStream* stream, void* push_constants, size_t pc_size) {
    if (!vk || !op || !stream || !stream->data) return;
    
    // Calculate total physical size (including all atoms and strides)
    size_t total_size = stream->count * stream->stride;
    
    // Ensure GPU buffer is ready
    if (!op->gpu_pipeline) tsfi_shader_pipeline_create(vk, op, pc_size);
    tsfi_shader_buffer_ensure(vk, op, total_size);
    
    // Upload data (preserving strides for hardware-aligned segments)
    if (op->gpu_mapped_ptr) {
        memcpy(op->gpu_mapped_ptr, stream->data, total_size);
    }
    
    // Execute
    tsfi_shader_pipeline_execute(vk, op, stream->data, total_size, push_constants, pc_size);
}
