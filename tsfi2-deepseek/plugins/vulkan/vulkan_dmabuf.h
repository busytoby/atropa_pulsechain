#ifndef VULKAN_DMABUF_H
#define VULKAN_DMABUF_H

#include "vulkan_system.h"

extern const struct zwp_linux_dmabuf_feedback_v1_listener feedback_listener;

struct wl_buffer* create_dma_buffer(VulkanSystem *s, int width, int height);

#endif
