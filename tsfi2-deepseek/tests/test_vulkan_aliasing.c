#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "window_inc/vulkan_struct.h"
#include "lau_memory.h"

// Function declared in plugins/vulkan_init.c
VulkanContext *init_vulkan(void);
void cleanup_vulkan(VulkanContext *vk);
void tag_vulkan_object(VulkanContext *vk, uint64_t handle, VkObjectType type, const char *name);

int main() {
    printf("=== TSFi Vulkan Resource Aliasing Validation ===\n");
    
    VulkanContext *vk = init_vulkan();
    if (!vk) return 1;

    // 1. Create one memory block
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = 4096,
        .memoryTypeIndex = 0 
    };
    
    for (uint32_t i = 0; i < vk->memory_properties.memoryTypeCount; i++) {
        if (vk->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            allocInfo.memoryTypeIndex = i;
            break;
        }
    }

    VkDeviceMemory memory;
    vk->vkAllocateMemory(vk->device, &allocInfo, NULL, &memory);
    tag_vulkan_object(vk, (uint64_t)memory, VK_OBJECT_TYPE_DEVICE_MEMORY, "Shared_Aliased_Memory");

    // 2. Create two buffers
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = 1024,
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
    };
    
    VkBuffer bufferA, bufferB;
    vk->vkCreateBuffer(vk->device, &bufferInfo, NULL, &bufferA);
    vk->vkCreateBuffer(vk->device, &bufferInfo, NULL, &bufferB);
    tag_vulkan_object(vk, (uint64_t)bufferA, VK_OBJECT_TYPE_BUFFER, "Rod_Buffer_Aliased");
    tag_vulkan_object(vk, (uint64_t)bufferB, VK_OBJECT_TYPE_BUFFER, "Cone_Buffer_Aliased");

    // 3. Bind both to the same memory range (Aliasing)
    printf("[VRS] Aliasing Rod and Cone buffers to same memory range...\n");
    vk->vkBindBufferMemory(vk->device, bufferA, memory, 0);
    vk->vkBindBufferMemory(vk->device, bufferB, memory, 0);

    // 4. Cleanup
    vk->vkDestroyBuffer(vk->device, bufferA, NULL);
    vk->vkDestroyBuffer(vk->device, bufferB, NULL);
    vk->vkFreeMemory(vk->device, memory, NULL);

    cleanup_vulkan(vk);
    printf("=== Aliasing Validation Complete ===\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}