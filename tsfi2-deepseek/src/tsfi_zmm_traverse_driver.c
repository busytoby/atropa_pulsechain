#include "tsfi_spirv_kernels.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

// Unified Vulkan Dispatcher for composite dual-GGUF weight traversal (DeepSeek + Stable Diffusion)
// Configuration: Emitter-Follower (unity voltage gain buffer reproducing input register signals to drive low-impedance output buffers)
void tsfi_zmm_dispatch_composite_traverse(
    VkDevice device,
    VkQueue queue,
    VkCommandPool cmdPool,
    VkBuffer regBuffer,
    VkBuffer deepseekWeightBuffer,
    VkBuffer sdWeightBuffer,
    VkBuffer outputBuffer,
    uint32_t workgroupCount
) {
    VkResult res;

    // 1. Create Shader Module from compiled SPIR-V constant
    VkShaderModuleCreateInfo shCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = zmm_traverse_spv_size,
        .pCode = zmm_traverse_spv
    };
    VkShaderModule shaderModule;
    res = vkCreateShaderModule(device, &shCreateInfo, NULL, &shaderModule);
    if (res != VK_SUCCESS) {
        fprintf(stderr, "[ZMM COMPOSITE ERROR] Failed to create shader module: %d\n", res);
        return;
    }

    // 2. Define Descriptor Set Layout (4 Bindings: registers, DeepSeek GGUF, SD GGUF, output vertices)
    VkDescriptorSetLayoutBinding bindings[4] = {
        {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
        },
        {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
        },
        {
            .binding = 2,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
        },
        {
            .binding = 3,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
        }
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 4,
        .pBindings = bindings
    };
    VkDescriptorSetLayout descLayout;
    res = vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descLayout);
    if (res != VK_SUCCESS) {
        vkDestroyShaderModule(device, shaderModule, NULL);
        return;
    }

    // 3. Create Pipeline Layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &descLayout
    };
    VkPipelineLayout pipelineLayout;
    res = vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout);
    if (res != VK_SUCCESS) {
        vkDestroyDescriptorSetLayout(device, descLayout, NULL);
        vkDestroyShaderModule(device, shaderModule, NULL);
        return;
    }

    // 4. Create Compute Pipeline
    VkComputePipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = shaderModule,
            .pName = "main"
        },
        .layout = pipelineLayout
    };
    VkPipeline pipeline;
    res = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &pipeline);
    if (res != VK_SUCCESS) {
        vkDestroyPipelineLayout(device, pipelineLayout, NULL);
        vkDestroyDescriptorSetLayout(device, descLayout, NULL);
        vkDestroyShaderModule(device, shaderModule, NULL);
        return;
    }

    // 5. Descriptor Pool & Set Allocation
    VkDescriptorPoolSize poolSize = {
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 4
    };
    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };
    VkDescriptorPool descPool;
    res = vkCreateDescriptorPool(device, &poolInfo, NULL, &descPool);
    if (res != VK_SUCCESS) {
        vkDestroyPipeline(device, pipeline, NULL);
        vkDestroyPipelineLayout(device, pipelineLayout, NULL);
        vkDestroyDescriptorSetLayout(device, descLayout, NULL);
        vkDestroyShaderModule(device, shaderModule, NULL);
        return;
    }

    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descLayout
    };
    VkDescriptorSet descSet;
    res = vkAllocateDescriptorSets(device, &allocInfo, &descSet);
    if (res != VK_SUCCESS) {
        vkDestroyDescriptorPool(device, descPool, NULL);
        vkDestroyPipeline(device, pipeline, NULL);
        vkDestroyPipelineLayout(device, pipelineLayout, NULL);
        vkDestroyDescriptorSetLayout(device, descLayout, NULL);
        vkDestroyShaderModule(device, shaderModule, NULL);
        return;
    }

    // 6. Update Descriptor Set with composite Buffers
    VkDescriptorBufferInfo bufferInfos[4] = {
        { .buffer = regBuffer, .offset = 0, .range = VK_WHOLE_SIZE },
        { .buffer = deepseekWeightBuffer, .offset = 0, .range = VK_WHOLE_SIZE },
        { .buffer = sdWeightBuffer, .offset = 0, .range = VK_WHOLE_SIZE },
        { .buffer = outputBuffer, .offset = 0, .range = VK_WHOLE_SIZE }
    };

    VkWriteDescriptorSet writes[4];
    for (int i = 0; i < 4; i++) {
        writes[i] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descSet,
            .dstBinding = i,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pBufferInfo = &bufferInfos[i]
        };
    }
    vkUpdateDescriptorSets(device, 4, writes, 0, NULL);

    // 7. Command Buffer Allocation & Recording
    VkCommandBufferAllocateInfo cmdAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cmdPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VkCommandBuffer cmdBuffer;
    res = vkAllocateCommandBuffers(device, &cmdAllocInfo, &cmdBuffer);
    if (res == VK_SUCCESS) {
        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };
        vkBeginCommandBuffer(cmdBuffer, &beginInfo);

        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descSet, 0, NULL);
        vkCmdDispatch(cmdBuffer, workgroupCount, 1, 1);

        vkEndCommandBuffer(cmdBuffer);

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmdBuffer
        };
        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuffer);
    }

    // Cleanup resources
    vkDestroyDescriptorPool(device, descPool, NULL);
    vkDestroyPipeline(device, pipeline, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyDescriptorSetLayout(device, descLayout, NULL);
    vkDestroyShaderModule(device, shaderModule, NULL);
}
