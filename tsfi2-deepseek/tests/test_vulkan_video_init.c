#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>

int main() {
    printf("=== TSFi Vulkan Video Encode Init Test ===\n");

    VkInstance instance;
    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "TSFiVideoTest",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "TSFi",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3,
    };
    VkInstanceCreateInfo inst_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
    };
    if (vkCreateInstance(&inst_info, NULL, &instance) != VK_SUCCESS) {
        printf("[FAIL] Failed to create Vulkan instance.\n");
        return 1;
    }

    uint32_t gpu_count = 0;
    vkEnumeratePhysicalDevices(instance, &gpu_count, NULL);
    if (gpu_count == 0) {
        printf("[FAIL] No Vulkan GPUs found.\n");
        vkDestroyInstance(instance, NULL);
        return 1;
    }

    VkPhysicalDevice *gpus = malloc(sizeof(VkPhysicalDevice) * gpu_count);
    vkEnumeratePhysicalDevices(instance, &gpu_count, gpus);

    bool found_video = false;
    for (uint32_t i = 0; i < gpu_count; i++) {
        uint32_t queue_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queue_count, NULL);
        
        VkQueueFamilyProperties *props = malloc(sizeof(VkQueueFamilyProperties) * queue_count);
        vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queue_count, props);

        for (uint32_t j = 0; j < queue_count; j++) {
            // Check for VK_QUEUE_VIDEO_ENCODE_BIT_KHR (0x00000040)
            if (props[j].queueFlags & 0x00000040) {
                printf("[PASS] Found Video Encode Queue on GPU %u family %u\n", i, j);
                found_video = true;
            }
        }
        free(props);
    }

    free(gpus);
    vkDestroyInstance(instance, NULL);

    if (!found_video) {
        printf("[FAIL] No Video Encode Queue found.\n");
        return 1;
    }

    printf("ALL VULKAN VIDEO INIT TESTS PASSED\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
