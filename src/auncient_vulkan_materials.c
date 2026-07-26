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
