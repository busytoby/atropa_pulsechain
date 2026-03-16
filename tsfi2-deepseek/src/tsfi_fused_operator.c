#include "tsfi_fused_operator.h"
#include "tsfi_shader_dispatch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TSFiFusedOp* tsfi_fused_create(const char* name) {
    TSFiFusedOp* fused = (TSFiFusedOp*)lau_malloc(sizeof(TSFiFusedOp));
    memset(fused, 0, sizeof(TSFiFusedOp));
    fused->name = lau_strdup(name);
    return fused;
}

void tsfi_fused_add(TSFiFusedOp* fused, TSFiOperator* op) {
    if (!fused || !op) return;
    fused->count++;
    fused->ops = (TSFiOperator**)lau_realloc(fused->ops, fused->count * sizeof(TSFiOperator*));
    fused->ops[fused->count - 1] = op;
}

void tsfi_fused_bake(TSFiFusedOp* fused, VulkanContext* vk, size_t buffer_size, void* push_constants, size_t pc_size) {
    if (!fused || !vk || fused->count == 0) return;
    if (fused->fused_cmd) vk->vkFreeCommandBuffers(vk->device, vk->command_pool, 1, &fused->fused_cmd);
    
    VkCommandBufferAllocateInfo cmdAlloc = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool = vk->command_pool, .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, .commandBufferCount = 1 };
    vk->vkAllocateCommandBuffers(vk->device, &cmdAlloc, &fused->fused_cmd);
    
    VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vk->vkBeginCommandBuffer(fused->fused_cmd, &beginInfo);
    
    VkMemoryBarrier barrier = { .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER, .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT, .dstAccessMask = VK_ACCESS_SHADER_READ_BIT };
    
    for (size_t i = 0; i < fused->count; i++) {
        TSFiOperator* op = fused->ops[i];
        if (!op->gpu_pipeline) tsfi_shader_pipeline_create(vk, op, pc_size);
        if (!op->gpu_buffer) tsfi_shader_buffer_ensure(vk, op, buffer_size);
        
        vk->vkCmdBindPipeline(fused->fused_cmd, VK_PIPELINE_BIND_POINT_COMPUTE, op->gpu_pipeline);
        vk->vkCmdBindDescriptorSets(fused->fused_cmd, VK_PIPELINE_BIND_POINT_COMPUTE, op->gpu_layout, 0, 1, &op->gpu_set, 0, NULL);
        if (pc_size > 0 && vk->vkCmdPushConstants) vk->vkCmdPushConstants(fused->fused_cmd, op->gpu_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, (uint32_t)pc_size, push_constants);
        vk->vkCmdDispatch(fused->fused_cmd, (uint32_t)(buffer_size / 64) + 1, 1, 1);
        
        if (i < fused->count - 1) {
            vk->vkCmdPipelineBarrier(fused->fused_cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &barrier, 0, NULL, 0, NULL);
        }
    }
    vk->vkEndCommandBuffer(fused->fused_cmd);
}

void tsfi_fused_dispatch(TSFiFusedOp* fused, VulkanContext* vk, WaveStream* stream) {
    if (!fused || !stream) return;
    size_t data_size = stream->count * stream->stride;
    
    if (vk && fused->count > 0) {
        // 1. Initial Data Transfer (To first op buffer)
        TSFiOperator* first = fused->ops[0];
        if (!first->gpu_buffer) tsfi_shader_buffer_ensure(vk, first, data_size);
        if (first->gpu_mapped_ptr) memcpy(first->gpu_mapped_ptr, stream->data, data_size);
        
        // 2. Bake if needed
        if (!fused->fused_cmd) {
            float dummy[2] = {1.0f, 0.0f};
            tsfi_fused_bake(fused, vk, data_size, &dummy, sizeof(dummy));
        }
        
        // 3. Submit Chain
        VkSubmitInfo submitInfo = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &fused->fused_cmd };
        vk->vkQueueSubmit(vk->queue, 1, &submitInfo, VK_NULL_HANDLE);
        vk->vkQueueWaitIdle(vk->queue);
        
        // 4. Retrieve Final Result
        TSFiOperator* last = fused->ops[fused->count - 1];
        if (last->gpu_mapped_ptr) memcpy(stream->data, last->gpu_mapped_ptr, data_size);
    } else {
        for (size_t i = 0; i < fused->count; i++) {
            TSFiOperator* op = fused->ops[i];
            if (op->cpu_entry) {
                void (*fn)(const WaveStream*, void*) = (void (*)(const WaveStream*, void*))op->cpu_entry;
                float dummy[2] = {1.0f, 0.0f}; fn(stream, &dummy);
            }
        }
    }
}

void tsfi_fused_destroy(TSFiFusedOp* fused, VulkanContext* vk) {
    if (!fused) return;
    if (vk && fused->fused_cmd) vk->vkFreeCommandBuffers(vk->device, vk->command_pool, 1, &fused->fused_cmd);
    if (fused->name) lau_free(fused->name);
    if (fused->ops) lau_free(fused->ops);
    lau_free(fused);
}
