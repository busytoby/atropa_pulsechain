// Append to plugins/vulkan/vulkan_dmabuf.c
#include "vulkan_dmabuf.h"
#include <stdio.h>

extern const VkAllocationCallbacks tsfi_alloc_callbacks;

static void init_staging_vk_buffer(VulkanContext *vk) {
    if (!vk || !vk->device) return;
    printf("[VULKAN] Initializing Staging Buffer (TSFi Zhong)... Auncient memory pipeline setup\n");

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = 32 * 1024 * 1024, // 32MB staging buffer
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if (vk->vkCreateBuffer(vk->device, &bufferInfo, &tsfi_alloc_callbacks, &vk->staging_buffer) != VK_SUCCESS) {
        printf("[VULKAN] Failed to create staging VkBuffer\n");
        return;
    }

    VkMemoryRequirements memReqs;
    vk->vkGetBufferMemoryRequirements(vk->device, vk->staging_buffer, &memReqs);

    int memTypeIndex = -1;
    for (uint32_t i = 0; i < vk->memory_properties.memoryTypeCount; i++) {
        if ((memReqs.memoryTypeBits & (1 << i)) && 
            (vk->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
            (vk->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            memTypeIndex = (int)i;
            break;
        }
    }

    if (memTypeIndex == -1) {
        vk->vkDestroyBuffer(vk->device, vk->staging_buffer, &tsfi_alloc_callbacks);
        vk->staging_buffer = VK_NULL_HANDLE;
        printf("[VULKAN] Failed to find coherent memory type for staging buffer\n");
        return;
    }

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = (uint32_t)memTypeIndex
    };

    if (vk->vkAllocateMemory(vk->device, &allocInfo, &tsfi_alloc_callbacks, &vk->staging_memory) != VK_SUCCESS) {
        vk->vkDestroyBuffer(vk->device, vk->staging_buffer, &tsfi_alloc_callbacks);
        vk->staging_buffer = VK_NULL_HANDLE;
        printf("[VULKAN] Failed to allocate memory for staging buffer\n");
        return;
    }

    vk->vkBindBufferMemory(vk->device, vk->staging_buffer, vk->staging_memory, 0);
    printf("[VULKAN] Successfully initialized staging buffer of size %zu\n", (size_t)memReqs.size);
}
