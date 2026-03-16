#ifndef VULKAN_INIT_H
#define VULKAN_INIT_H

#include "window_inc/vulkan_struct.h"

VulkanContext* init_vulkan(void);
VulkanContext* init_vulkan_display(void); // KMS/VTY Backend
void cleanup_vulkan(VulkanContext *vk);
void init_vk_swapchain(VulkanContext *vk, int width, int height);

#endif