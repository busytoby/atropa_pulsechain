#ifndef AUNCIENT_VULKAN_MATERIALS_H
#define AUNCIENT_VULKAN_MATERIALS_H

#include <stdint.h>

// Auncient Vulkan Scope-Based Material Descriptor Layouts

// Descriptor Set 0: Binds once per frame (Global context)
typedef struct {
    float projection_matrix[16];
    float view_matrix[16];
    float camera_position[3];
    float global_time;
} GlobalUniformBlock;

// Descriptor Set 1: Binds once per material group (Material template constants)
typedef struct {
    uint32_t seed;
    float persistence;
    float lacunarity;
    uint32_t octaves;
    uint32_t color_preset;
    float material_blend_factor;
} MaterialUniformBlock;

// Descriptor Set 2: Binds once per draw call (Object instance variables)
typedef struct {
    float local_transform[16];
    float joint_swing_angle;
    uint32_t instance_id;
} InstanceUniformBlock;

// Mock Vulkan types to support sandbox compile verification
typedef enum {
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6,
    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = 1
} VkDescriptorType;

typedef enum {
    VK_SHADER_STAGE_VERTEX_BIT = 0x00000001,
    VK_SHADER_STAGE_FRAGMENT_BIT = 0x00000002,
    VK_SHADER_STAGE_ALL_GRAPHICS = 0x0000001F
} VkShaderStageFlags;

typedef struct {
    uint32_t binding;
    VkDescriptorType descriptorType;
    uint32_t descriptorCount;
    VkShaderStageFlags stageFlags;
    const void* pImmutableSamplers;
} VkDescriptorSetLayoutBinding;

typedef struct {
    uint32_t sType;
    const void* pNext;
    uint32_t flags;
    uint32_t bindingCount;
    const VkDescriptorSetLayoutBinding* pBindings;
} VkDescriptorSetLayoutCreateInfo;

typedef struct {
    uint32_t sType;
    const void* pNext;
    uint32_t flags;
    uint32_t setLayoutCount;
    const void* pSetLayouts; // Mock VkDescriptorSetLayout handle array
    uint32_t pushConstantRangeCount;
    const void* pPushConstantRanges;
} VkPipelineLayoutCreateInfo;

#ifdef __cplusplus
extern "C" {
#endif

void auncient_vulkan_materials_get_bindings(int set_index, VkDescriptorSetLayoutBinding *bindings, int *count);
void auncient_vulkan_materials_build_layout_info(const VkDescriptorSetLayoutBinding *bindings, int count, VkDescriptorSetLayoutCreateInfo *info);
void auncient_vulkan_materials_build_pipeline_layout(const void *set_layouts, int count, VkPipelineLayoutCreateInfo *info);

#ifdef __cplusplus
}
#endif

#endif // AUNCIENT_VULKAN_MATERIALS_H
