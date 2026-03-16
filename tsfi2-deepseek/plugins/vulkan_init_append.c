// Append to plugins/vulkan_init.c

void init_swapchain(VulkanContext *vk, int width, int height) {
    if (!vk || !vk->device || !vk->surface) return;
    
    printf("[VULKAN] Initializing Swapchain (%dx%d)...
", width, height);
    
    VkSurfaceCapabilitiesKHR caps;
    vk->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk->physical_device, vk->surface, &caps);
    
    VkSwapchainCreateInfoKHR swapInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = vk->surface,
        .minImageCount = caps.minImageCount + 1,
        .imageFormat = VK_FORMAT_B8G8R8A8_SRGB, // Standard Wayland
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = { (uint32_t)width, (uint32_t)height },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = caps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR, // V-Sync
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };
    
    if (vk->vkCreateSwapchainKHR(vk->device, &swapInfo, NULL, &vk->swapchain) != VK_SUCCESS) {
        printf("[VULKAN] Failed to create swapchain.
");
        return;
    }
    
    vk->vkGetSwapchainImagesKHR(vk->device, vk->swapchain, &vk->swapchainImageCount, NULL);
    vk->swapchainImages = lau_malloc(sizeof(VkImage) * vk->swapchainImageCount);
    vk->vkGetSwapchainImagesKHR(vk->device, vk->swapchain, &vk->swapchainImageCount, vk->swapchainImages);
    
    vk->swapchainImageViews = lau_malloc(sizeof(VkImageView) * vk->swapchainImageCount);
    for (uint32_t i = 0; i < vk->swapchainImageCount; i++) {
        VkImageViewCreateInfo viewInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = vk->swapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
        };
        vk->vkCreateImageView(vk->device, &viewInfo, NULL, &vk->swapchainImageViews[i]);
    }
    printf("[VULKAN] Swapchain Ready (%d images).
", vk->swapchainImageCount);
}

void init_sync_objects(VulkanContext *vk) {
    if (!vk || !vk->device) return;
    
    VkSemaphoreCreateInfo semInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceInfo = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT };
    
    vk->vkCreateSemaphore(vk->device, &semInfo, NULL, &vk->imageAvailableSemaphore);
    vk->vkCreateSemaphore(vk->device, &semInfo, NULL, &vk->renderFinishedSemaphore);
    vk->vkCreateFence(vk->device, &fenceInfo, NULL, &vk->inFlightFence);
    
    // Timeline Semaphore for Compute/Transfer Sync (Zhong)
    VkSemaphoreTypeCreateInfo timelineInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = 0
    };
    VkSemaphoreCreateInfo tSemInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &timelineInfo
    };
    vk->vkCreateSemaphore(vk->device, &tSemInfo, NULL, &vk->timelineSemaphore);
    
    printf("[VULKAN] Sync Objects Initialized (Timeline Active).
");
}
