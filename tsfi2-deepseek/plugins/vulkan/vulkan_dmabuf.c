#include "vulkan_dmabuf.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// --- DMABUF Feedback REMOVED (Source of Historical Hangs) ---

struct wl_buffer* create_dma_buffer(VulkanSystem *s, int width, int height) {
    VulkanContext *vk = s->vk;
    if (!vk || !vk->vkGetMemoryFdKHR || !s->dmabuf) return NULL;

    VkExternalMemoryImageCreateInfo extInfo = {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
        .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT
    };

    VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = &extInfo,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_B8G8R8A8_UNORM,
        .extent = { (uint32_t)width, (uint32_t)height, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_LINEAR,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    
    VkImage image;
    if (vk->vkCreateImage(vk->device, &imageInfo, &tsfi_alloc_callbacks, &image) != VK_SUCCESS) {
        printf("[TSFI_VULKAN] Failed to create external image.\n");
        return NULL;
    }
    s->dma_image = image;

    VkMemoryRequirements memReqs;
    vk->vkGetImageMemoryRequirements(vk->device, image, &memReqs);
    
    int memTypeIndex = -1;
    for (uint32_t i = 0; i < vk->memory_properties.memoryTypeCount; i++) {
        if ((memReqs.memoryTypeBits & (1 << i)) && (vk->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
            memTypeIndex = (int)i;
            break;
        }
    }
    
    if (memTypeIndex == -1) {
        vk->vkDestroyImage(vk->device, image, &tsfi_alloc_callbacks);
        s->dma_image = VK_NULL_HANDLE;
        return NULL;
    }

    VkExportMemoryAllocateInfo exportAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO,
        .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT
    };
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &exportAllocInfo,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = (uint32_t)memTypeIndex
    };
    
    VkDeviceMemory memory;
    if (vk->vkAllocateMemory(vk->device, &allocInfo, &tsfi_alloc_callbacks, &memory) != VK_SUCCESS) {
        vk->vkDestroyImage(vk->device, image, &tsfi_alloc_callbacks);
        s->dma_image = VK_NULL_HANDLE;
        return NULL;
    }
    s->dma_mem = memory;

    vk->vkBindImageMemory(vk->device, image, memory, 0);
    
    VkMemoryGetFdInfoKHR fdInfo;
    memset(&fdInfo, 0, sizeof(fdInfo));
    fdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
    fdInfo.pNext = NULL;
    fdInfo.memory = memory;
    fdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    
    int fd = -1;
    if (vk->vkGetMemoryFdKHR(vk->device, &fdInfo, &fd) != VK_SUCCESS) {
        vk->vkFreeMemory(vk->device, memory, &tsfi_alloc_callbacks);
        vk->vkDestroyImage(vk->device, image, &tsfi_alloc_callbacks);
        s->dma_mem = VK_NULL_HANDLE;
        s->dma_image = VK_NULL_HANDLE;
        return NULL;
    }

    VkImageSubresource sub = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .arrayLayer = 0 };
    VkSubresourceLayout layout;
    vk->vkGetImageSubresourceLayout(vk->device, image, &sub, &layout);

    struct zwp_linux_buffer_params_v1 *params = zwp_linux_dmabuf_v1_create_params(s->dmabuf);
    zwp_linux_buffer_params_v1_add(params, fd, 0, (uint32_t)layout.offset, (uint32_t)layout.rowPitch, 0, 0);
    struct wl_buffer *buffer = zwp_linux_buffer_params_v1_create_immed(params, width, height, 0x34325258, 0);
    zwp_linux_buffer_params_v1_destroy(params);
    
    close(fd);
    
    printf("[TSFI_VULKAN] DMABUF Buffer Created (Direct Creation Path).\n");
    return buffer;
}
