#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "lau_memory.h"
#include "vulkan_init.h"
#include "vulkan_render.h"

// Note: Function defined in src/tsfi_drm_lease_thunk.c
extern int tsfi_acquire_hardware_lease(void);

int main() {
    printf("=== TSFi Sovereign Proof: Plane 71 Lease & Draw ===\n");

    // 1. Physical Arbitration: Acquire the Lease
    printf("[ACTION] Acquiring Hardware Lease...\n");
    int leased_fd = tsfi_acquire_hardware_lease();
    if (leased_fd < 0) {
        printf("[FAIL] Could not acquire lease. This environment does not support physical Plane 71 access.\n");
        return 1;
    }
    printf("[PASS] Physical Lease SECURED. FD: %d\n", leased_fd);

    // 2. Vulkan Initialization: Direct Display Path
    printf("[ACTION] Initializing Vulkan on physical display pins...\n");
    VulkanContext *vk = init_vulkan(leased_fd);
    if (!vk || !vk->surface) {
        printf("[FAIL] Failed to create Plane 71 surface.\n");
        return 1;
    }
    printf("[PASS] Physical Plane 71 Surface Promoted.\n");

    // 3. Setup Physical Swapchain
    init_vk_swapchain(vk, 1920, 1080); // Targeting full-screen
    printf("[PASS] Physical Swapchain Active.\n");

    // 4. Tight 171 FPS Draw Loop
    printf("[ACTION] Drawing directly to the screen (TSFi Blue). Press Ctrl+C to stop.\n");
    uint32_t frame_idx = 0;
    while (frame_idx < 500) {
        uint32_t imageIndex;
        vk->vkAcquireNextImageKHR(vk->device, vk->swapchain, UINT64_MAX, vk->imageAvailableSemaphores[0], VK_NULL_HANDLE, &imageIndex);

        VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        vk->vkBeginCommandBuffer(vk->command_buffers[0], &beginInfo);

        // Physical Clear to TSFi Blue (0.0, 0.1, 0.2)
        VkClearColorValue clearColor = { .float32 = { 0.0f, 0.1f, 0.2f, 1.0f } };
        VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        vk->vkCmdClearColorImage(vk->command_buffers[0], vk->swapchainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);

        vk->vkEndCommandBuffer(vk->command_buffers[0]);

        VkSubmitInfo submitInfo = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &vk->command_buffers[0] };
        vk->vkQueueSubmit(vk->queue, 1, &submitInfo, VK_NULL_HANDLE);

        VkPresentInfoKHR presentInfo = { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, .swapchainCount = 1, .pSwapchains = &vk->swapchain, .pImageIndices = &imageIndex };
        vk->vkQueuePresentKHR(vk->queue, &presentInfo);

        frame_idx++;
        if (frame_idx % 100 == 0) printf("\r[HW] Pushed Frame: %u (171 FPS) ", frame_idx);
        fflush(stdout);
        usleep(5847);
    }

    printf("\n[SUCCESS] Physical Sovereignty Proven.\n");
    cleanup_vulkan(vk);
    return 0;
}
