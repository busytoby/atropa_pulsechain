#include "../tsfi2-deepseek/src/auncient_vulkan_materials.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT VULKAN MATERIALS LAYOUT TEST\n");
    printf("=============================================================\n");

    VkDescriptorSetLayoutBinding bindings[5];
    int count = 0;

    // Test Set 0 (Global)
    auncient_vulkan_materials_get_bindings(0, bindings, &count);
    assert(count == 1);
    assert(bindings[0].binding == 0);
    assert(bindings[0].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    assert(bindings[0].stageFlags == VK_SHADER_STAGE_ALL_GRAPHICS);
    printf("   ✓ Set 0 (Global) layout bindings verified.\n");

    // Test Set 1 (Material)
    auncient_vulkan_materials_get_bindings(1, bindings, &count);
    assert(count == 1);
    assert(bindings[0].binding == 0);
    assert(bindings[0].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    assert(bindings[0].stageFlags == VK_SHADER_STAGE_FRAGMENT_BIT);
    printf("   ✓ Set 1 (Material template) layout bindings verified.\n");

    // Test Set 2 (Instance)
    auncient_vulkan_materials_get_bindings(2, bindings, &count);
    assert(count == 1);
    assert(bindings[0].binding == 0);
    assert(bindings[0].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    assert(bindings[0].stageFlags == VK_SHADER_STAGE_VERTEX_BIT);
    printf("   ✓ Set 2 (Instance variables) layout bindings verified.\n");

    // Test Descriptor Set Layout Info building
    VkDescriptorSetLayoutCreateInfo layout_info;
    auncient_vulkan_materials_build_layout_info(bindings, count, &layout_info);
    assert(layout_info.sType == 8);
    assert(layout_info.bindingCount == 1);
    assert(layout_info.pBindings == bindings);
    printf("   ✓ Descriptor set layout builder verified.\n");

    // Test Pipeline Layout Info building with Push Constants
    VkPushConstantRange push_range;
    auncient_vulkan_materials_get_push_constant_range(&push_range);
    assert(push_range.stageFlags == (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT));
    assert(push_range.offset_bytes == 0);
    assert(push_range.size == sizeof(MaterialPushConstants));
    printf("   ✓ Push constant range builder verified.\n");

    void* mock_set_layouts[3] = {(void*)0x1, (void*)0x2, (void*)0x3};
    VkPipelineLayoutCreateInfo pipeline_layout_info;
    auncient_vulkan_materials_build_pipeline_layout(mock_set_layouts, 3, &push_range, 1, &pipeline_layout_info);
    assert(pipeline_layout_info.sType == 30);
    assert(pipeline_layout_info.setLayoutCount == 3);
    assert(pipeline_layout_info.pSetLayouts == (const void*)mock_set_layouts);
    assert(pipeline_layout_info.pushConstantRangeCount == 1);
    assert(pipeline_layout_info.pPushConstantRanges == &push_range);
    printf("   ✓ Pipeline layout builder verified.\n");

    // Test Size and Alignment Estimator
    uint32_t raw_sz = 0;
    uint32_t aligned_sz = 0;
    auncient_vulkan_materials_get_sizes(0, 256, &raw_sz, &aligned_sz);
    assert(raw_sz == sizeof(GlobalUniformBlock));
    assert(aligned_sz == 256); // 132 bytes rounded up to 256 alignment
    printf("   ✓ Size and alignment estimators verified.\n");

    // Test Descriptor Buffer Info & Write Set building
    VkDescriptorBufferInfo buf_info;
    void* mock_buffer = (void*)0xABCD;
    auncient_vulkan_materials_build_buffer_info(mock_buffer, 256, 128, &buf_info);
    assert(buf_info.buffer == mock_buffer);
    assert(buf_info.offset_bytes == 256);
    assert(buf_info.range == 128);
    printf("   ✓ Descriptor buffer info builder verified.\n");

    VkWriteDescriptorSet write_set;
    void* mock_set = (void*)0x9999;
    auncient_vulkan_materials_build_write_set(mock_set, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &buf_info, &write_set);
    assert(write_set.sType == 35);
    assert(write_set.dstSet == mock_set);
    assert(write_set.dstBinding == 0);
    assert(write_set.pBufferInfo == &buf_info);
    printf("   ✓ Write descriptor set builder verified.\n");

    // Test Descriptor Pool creation builders
    VkDescriptorPoolSize pool_sizes[2];
    int pool_size_cnt = 0;
    auncient_vulkan_materials_build_pool_sizes(10, 20, pool_sizes, &pool_size_cnt);
    assert(pool_size_cnt == 1);
    assert(pool_sizes[0].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    assert(pool_sizes[0].descriptorCount == 31); // 1 (Global) + 10 (Materials) + 20 (Instances)
    printf("   ✓ Descriptor pool sizes builder verified.\n");

    VkDescriptorPoolCreateInfo pool_info;
    auncient_vulkan_materials_build_pool_info(35, pool_sizes, pool_size_cnt, &pool_info);
    assert(pool_info.sType == 33);
    assert(pool_info.maxSets == 35);
    assert(pool_info.poolSizeCount == 1);
    assert(pool_info.pPoolSizes == pool_sizes);
    printf("   ✓ Descriptor pool info builder verified.\n");

    // Test Descriptor Set Allocation info builder
    VkDescriptorSetAllocateInfo alloc_info;
    void* mock_pool = (void*)0x7777;
    auncient_vulkan_materials_build_allocate_info(mock_pool, mock_set_layouts, 3, &alloc_info);
    assert(alloc_info.sType == 9);
    assert(alloc_info.descriptorPool == mock_pool);
    assert(alloc_info.descriptorSetCount == 3);
    assert(alloc_info.pSetLayouts == mock_set_layouts);
    printf("   ✓ Descriptor set allocation info builder verified.\n");

    printf("=============================================================\n");
    printf("AUNCIENT VULKAN MATERIALS TEST COMPLETE\n");
    printf("=============================================================\n");
    return 0;
}
