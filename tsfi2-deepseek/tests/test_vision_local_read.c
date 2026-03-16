#include "window_inc/vulkan_struct.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * TSFi Vision: Dynamic Rendering Local Read Validation
 * 
 * Verifies that the hardware supports reading directly from color attachments 
 * in the fragment shader, enabling iterative vision algorithms.
 */

// Function declared in plugins/vulkan_init.c
VulkanContext *init_vulkan(void);
void cleanup_vulkan(VulkanContext *vk);

int main() {
    printf("=== TSFi Vision: Local Read Validation (Vulkan Roadmap) ===\n");
    
    VulkanContext *vk = init_vulkan();
    if (!vk) return 1;

    // Verify Feature Support (Programmatically)
    VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR localReadFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR
    };
    VkPhysicalDeviceVulkan13Features features13 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &localReadFeatures
    };
    VkPhysicalDeviceFeatures2 features2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &features13
    };

    vk->vkGetPhysicalDeviceFeatures2(vk->physical_device, &features2);

    printf("[VISION] Feature: dynamicRenderingLocalRead -> %s\n", 
           localReadFeatures.dynamicRenderingLocalRead ? "SUPPORTED" : "UNSUPPORTED");

    if (vk->vkCmdSetRenderingAttachmentLocations && vk->vkCmdSetRenderingInputAttachmentIndices) {
        printf("[VISION] Local Read Commands -> LOADED\n");
    } else {
        printf("[VISION] Local Read Commands -> MISSING\n");
    }

    if (localReadFeatures.dynamicRenderingLocalRead && vk->vkCmdSetRenderingAttachmentLocations) {
        printf("[PASS] Local Read Physical Link Active.\n");
    } else {
        printf("[FAIL] Local Read Requirements Not Met.\n");
    }

    cleanup_vulkan(vk);
    printf("=== Validation Complete ===\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}