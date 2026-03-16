#include "window_inc/vulkan_struct.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * TSFi Vision: Local Read Command Recording Benchmark
 * 
 * Measures the overhead of setting up local read attachment locations 
 * vs. standard dynamic rendering.
 */

// Function declared in plugins/vulkan_init.c
VulkanContext *init_vulkan(void);
void cleanup_vulkan(VulkanContext *vk);

double get_time_s() {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void bench_local_read_recording(VulkanContext *vk, int iterations) {
    if (!vk->vkCmdSetRenderingAttachmentLocations || !vk->vkCmdBeginRendering) {
        printf("  [SKIP] Local Read or Dynamic Rendering Commands not supported.\n");
        return;
    }

    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vk->command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VkCommandBuffer cmd;
    vk->vkAllocateCommandBuffers(vk->device, &allocInfo, &cmd);

    uint32_t locations[] = {0};
    VkRenderingAttachmentLocationInfo locationInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_LOCATION_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentLocations = locations
    };

    // Minimal Dummy Attachment (Spec compliant)
    VkRenderingAttachmentInfo colorAttachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = VK_NULL_HANDLE, // Allowed if no actual load/store is performed or if placeholder
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE
    };

    // Minimal Rendering Setup
    VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = { {0, 0}, {1024, 1024} },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment
    };

    double start = get_time_s();
    for (int i = 0; i < iterations; i++) {
        VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        vk->vkBeginCommandBuffer(cmd, &beginInfo);
        
        // Start Dynamic Rendering
        vk->vkCmdBeginRendering(cmd, &renderingInfo);

        // Dynamic Rendering Local Read Setup (Inside Render Pass)
        vk->vkCmdSetRenderingAttachmentLocations(cmd, &locationInfo);
        
        vk->vkCmdEndRendering(cmd);
        vk->vkEndCommandBuffer(cmd);
    }
    double end = get_time_s();

    printf("  Record Time (%d iter): %.4f s (%.2f us/record)\n", 
           iterations, end - start, (end - start) * 1e6 / iterations);

    vk->vkFreeCommandBuffers(vk->device, vk->command_pool, 1, &cmd);
}

int main() {
    printf("=== TSFi Vision: Local Read Recording Benchmark ===\n");
    
    VulkanContext *vk = init_vulkan();
    if (!vk) return 1;

    bench_local_read_recording(vk, 10000);

    cleanup_vulkan(vk);
    printf("=== Benchmark Complete ===\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}