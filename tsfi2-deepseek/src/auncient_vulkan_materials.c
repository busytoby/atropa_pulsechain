#include "auncient_vulkan_materials.h"
#include <string.h>

// Auncient Vulkan Scope-Based Material Descriptor Layouts - Implementation

void auncient_vulkan_materials_get_bindings(int set_index, VkDescriptorSetLayoutBinding *bindings, int *count) {
    if (!bindings || !count) return;

    if (set_index == 0) {
        // Set 0: Global context - binding 0 (Vertex & Fragment stages)
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
        bindings[0].pImmutableSamplers = NULL;
        *count = 1;
    } 
    else if (set_index == 1) {
        // Set 1: Material constants - binding 0 (Fragment stage for .dna parameters)
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[0].pImmutableSamplers = NULL;
        *count = 1;
    } 
    else if (set_index == 2) {
        // Set 2: Instance variables - binding 0 (Vertex stage for skeletal joint rotations)
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        bindings[0].pImmutableSamplers = NULL;
        *count = 1;
    } 
    else {
        *count = 0;
    }
}

void auncient_vulkan_materials_build_layout_info(const VkDescriptorSetLayoutBinding *bindings, int count, VkDescriptorSetLayoutCreateInfo *info) {
    if (!info) return;

    info->sType = 8; // VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
    info->pNext = NULL;
    info->flags = 0;
    info->bindingCount = (uint32_t)count;
    info->pBindings = bindings;
}

void auncient_vulkan_materials_get_push_constant_range(VkPushConstantRange *range) {
    if (!range) return;

    range->stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    range->offset_bytes = 0;
    range->size = sizeof(MaterialPushConstants);
}

void auncient_vulkan_materials_build_pipeline_layout(const void *set_layouts, int layout_count, const VkPushConstantRange *push_ranges, int push_count, VkPipelineLayoutCreateInfo *info) {
    if (!info) return;

    info->sType = 30; // VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
    info->pNext = NULL;
    info->flags = 0;
    info->setLayoutCount = (uint32_t)layout_count;
    info->pSetLayouts = set_layouts;
    info->pushConstantRangeCount = (uint32_t)push_count;
    info->pPushConstantRanges = push_ranges;
}

void auncient_vulkan_materials_get_sizes(int set_index, uint32_t gpu_alignment, uint32_t *raw_size, uint32_t *aligned_size) {
    if (!raw_size || !aligned_size) return;

    uint32_t size = 0;
    if (set_index == 0) {
        size = sizeof(GlobalUniformBlock);
    } else if (set_index == 1) {
        size = sizeof(MaterialUniformBlock);
    } else if (set_index == 2) {
        size = sizeof(InstanceUniformBlock);
    }

    *raw_size = size;
    
    if (gpu_alignment > 0) {
        // Round up to nearest gpu_alignment boundary
        *aligned_size = (size + gpu_alignment - 1) & ~(gpu_alignment - 1);
    } else {
        *aligned_size = size;
    }
}

void auncient_vulkan_materials_build_buffer_info(void *buffer, uint64_t displacement, uint64_t range, VkDescriptorBufferInfo *info) {
    if (!info) return;

    info->buffer = buffer;
    info->offset_bytes = displacement;
    info->range = range;
}

void auncient_vulkan_materials_build_write_set(void *dst_set, uint32_t binding, VkDescriptorType type, const VkDescriptorBufferInfo *buffer_info, VkWriteDescriptorSet *write_set) {
    if (!write_set) return;

    write_set->sType = 35; // VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    write_set->pNext = NULL;
    write_set->dstSet = dst_set;
    write_set->dstBinding = binding;
    write_set->dstArrayElement = 0;
    write_set->descriptorCount = 1;
    write_set->descriptorType = type;
    write_set->pImageInfo = NULL;
    write_set->pBufferInfo = buffer_info;
    write_set->pTexelBufferView = NULL;
}

void auncient_vulkan_materials_build_pool_sizes(int material_count, int instance_count, VkDescriptorPoolSize *sizes, int *size_count) {
    if (!sizes || !size_count) return;

    // We allocate 1 Uniform Buffer slot for Global context, N slots for Materials, and M slots for Instances
    sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    sizes[0].descriptorCount = (uint32_t)(1 + material_count + instance_count);
    *size_count = 1;
}

void auncient_vulkan_materials_build_pool_info(uint32_t max_sets, const VkDescriptorPoolSize *sizes, int size_count, VkDescriptorPoolCreateInfo *info) {
    if (!info) return;

    info->sType = 33; // VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO
    info->pNext = NULL;
    info->flags = 0;
    info->maxSets = max_sets;
    info->poolSizeCount = (uint32_t)size_count;
    info->pPoolSizes = sizes;
}

void auncient_vulkan_materials_build_allocate_info(void *pool, const void *set_layouts, int count, VkDescriptorSetAllocateInfo *info) {
    if (!info) return;

    info->sType = 9; // VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO
    info->pNext = NULL;
    info->descriptorPool = pool;
    info->descriptorSetCount = (uint32_t)count;
    info->pSetLayouts = set_layouts;
}
