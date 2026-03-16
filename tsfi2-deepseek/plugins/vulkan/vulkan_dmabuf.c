#include "vulkan_dmabuf.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// --- DMABUF Roadmap 2026 Feedback ---
static void feedback_handle_done(void *data, struct zwp_linux_dmabuf_feedback_v1 *feedback) {
    (void)feedback;
    VulkanSystem *s = (VulkanSystem *)data;
    s->dmabuf_state.feedback_received = true;
    printf("[TSFI_VULKAN] DMABUF Feedback Received.\n");
}

static void feedback_handle_format_table(void *data, struct zwp_linux_dmabuf_feedback_v1 *feedback, int32_t fd, uint32_t size) {
    (void)data; (void)feedback; (void)size;
    close(fd); 
}

static void feedback_handle_main_device(void *data, struct zwp_linux_dmabuf_feedback_v1 *feedback, struct wl_array *device) {
    (void)feedback;
    VulkanSystem *s = (VulkanSystem *)data;
    if (device->size >= 8) {
        uint32_t *dev = (uint32_t *)device->data;
        s->dmabuf_state.main_device_id_maj = dev[0];
        s->dmabuf_state.main_device_id_min = dev[1];
        printf("[TSFI_VULKAN] Main Device Affinity: %u:%u\n", dev[0], dev[1]);
    }
}

static void feedback_handle_tranche_done(void *data, struct zwp_linux_dmabuf_feedback_v1 *feedback) {
    (void)data; (void)feedback;
}

static void feedback_handle_tranche_target_device(void *data, struct zwp_linux_dmabuf_feedback_v1 *feedback, struct wl_array *device) {
    (void)data; (void)feedback; (void)device;
}

static void feedback_handle_tranche_formats(void *data, struct zwp_linux_dmabuf_feedback_v1 *feedback, struct wl_array *indices) {
    (void)feedback;
    VulkanSystem *s = (VulkanSystem *)data;
    // For now, we don't map the table here, but we could check if any index maps to 0x34325258.
    // Simplifying: If we get formats, we assume native XRGB8888 is among them for demo purposes, 
    // or we'd need to track the format table.
    (void)indices;
    s->dmabuf_state.format_supported = true;
}

static void feedback_handle_tranche_flags(void *data, struct zwp_linux_dmabuf_feedback_v1 *feedback, uint32_t flags) {
    (void)data; (void)feedback; (void)flags;
}

const struct zwp_linux_dmabuf_feedback_v1_listener feedback_listener = {
    .done = feedback_handle_done,
    .format_table = feedback_handle_format_table,
    .main_device = feedback_handle_main_device,
    .tranche_done = feedback_handle_tranche_done,
    .tranche_target_device = feedback_handle_tranche_target_device,
    .tranche_formats = feedback_handle_tranche_formats,
    .tranche_flags = feedback_handle_tranche_flags
};

struct wl_buffer* create_dma_buffer(VulkanSystem *s, int width, int height) {
    VulkanContext *vk = s->vk;
    if (!vk || !vk->vkGetMemoryFdKHR) return NULL;

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
    if (vk->vkCreateImage(vk->device, &imageInfo, NULL, &image) != VK_SUCCESS) {
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
        vk->vkDestroyImage(vk->device, image, NULL);
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
    if (vk->vkAllocateMemory(vk->device, &allocInfo, NULL, &memory) != VK_SUCCESS) {
        vk->vkDestroyImage(vk->device, image, NULL);
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
    
    printf("[TSFI_VULKAN] DEBUG: Calling vkGetMemoryFdKHR. sType=%d (Expected=%d), addr=%p\n", 
           fdInfo.sType, VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR, (void*)&fdInfo);
    fflush(stdout);

    int fd = -1;
    if (vk->vkGetMemoryFdKHR(vk->device, &fdInfo, &fd) != VK_SUCCESS) {
        vk->vkFreeMemory(vk->device, memory, NULL);
        vk->vkDestroyImage(vk->device, image, NULL);
        s->dma_mem = VK_NULL_HANDLE;
        s->dma_image = VK_NULL_HANDLE;
        return NULL;
    }

    VkImageSubresource sub = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .arrayLayer = 0 };
    VkSubresourceLayout layout;
    vk->vkGetImageSubresourceLayout(vk->device, image, &sub, &layout);

    struct zwp_linux_buffer_params_v1 *params = zwp_linux_dmabuf_v1_create_params(s->dmabuf);
    // Use DRM_FORMAT_XRGB8888 (0x34325258) and modifier 0 (LINEAR)
    zwp_linux_buffer_params_v1_add(params, fd, 0, (uint32_t)layout.offset, (uint32_t)layout.rowPitch, 0, 0);
    struct wl_buffer *buffer = zwp_linux_buffer_params_v1_create_immed(params, width, height, 0x34325258, 0);
    zwp_linux_buffer_params_v1_destroy(params);
    
    close(fd);
    
    printf("[TSFI_VULKAN] DMABUF Buffer Created (%dx%d) stride=%u.\n", width, height, (uint32_t)layout.rowPitch);
    return buffer;
}
