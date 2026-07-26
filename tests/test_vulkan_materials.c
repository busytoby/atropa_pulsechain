#include "../src/auncient_vulkan_materials.h"
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

    printf("=============================================================\n");
    printf("AUNCIENT VULKAN MATERIALS TEST COMPLETE\n");
    printf("=============================================================\n");
    return 0;
}
