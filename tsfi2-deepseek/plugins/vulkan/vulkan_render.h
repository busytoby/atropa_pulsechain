#ifndef VULKAN_RENDER_H
#define VULKAN_RENDER_H

#include "vulkan_system.h"

// Rendering Helpers
bool init_swapchain(VulkanSystem *s);
bool init_sync_objects(VulkanSystem *s);
bool init_staging_vk_buffer(VulkanSystem *s, size_t size);
void upload_staging_to_image(VulkanSystem *s, StagingBuffer *sb, VkImage target, VkCommandBuffer cmd);
void recreate_swapchain(VulkanSystem *s);
void draw_frame(VulkanSystem *s);
void draw_ui_elements(VulkanSystem *s);

#endif