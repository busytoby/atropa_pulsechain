#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "lau_memory.h"
#include "window_inc/vulkan_struct.h"

extern PFN_vkVoidFunction tsfi_vkGetInstanceProcAddr(VkInstance instance, const char* pName);

void test_firmware_drm_display() {
    printf("[TEST] Testing Firmware Internal DRM Display Leasing & Page Flips...\n");

    // Load necessary functions from TSFi Firmware ICD
    PFN_vkCreateInstance pvkCreateInstance = (PFN_vkCreateInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    PFN_vkEnumeratePhysicalDevices pvkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)tsfi_vkGetInstanceProcAddr(NULL, "vkEnumeratePhysicalDevices");
    PFN_vkGetPhysicalDeviceDisplayPropertiesKHR pvkGetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)tsfi_vkGetInstanceProcAddr(NULL, "vkGetPhysicalDeviceDisplayPropertiesKHR");
    PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR pvkGetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)tsfi_vkGetInstanceProcAddr(NULL, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    PFN_vkCreateDisplayPlaneSurfaceKHR pvkCreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateDisplayPlaneSurfaceKHR");
    
    PFN_vkCreateDevice pvkCreateDevice = (PFN_vkCreateDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateDevice");
    PFN_vkGetDeviceQueue pvkGetDeviceQueue = (PFN_vkGetDeviceQueue)tsfi_vkGetInstanceProcAddr(NULL, "vkGetDeviceQueue");
    PFN_vkCreateSwapchainKHR pvkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateSwapchainKHR");
    PFN_vkQueuePresentKHR pvkQueuePresentKHR = (PFN_vkQueuePresentKHR)tsfi_vkGetInstanceProcAddr(NULL, "vkQueuePresentKHR");

    // 1. Initialize Firmware Vulkan
    VkInstance instance = NULL;
    VkInstanceCreateInfo inst_info = {0};
    pvkCreateInstance(&inst_info, NULL, &instance);
    assert(instance != NULL);

    VkPhysicalDevice physicalDevice = NULL;
    uint32_t dev_count = 1;
    pvkEnumeratePhysicalDevices(instance, &dev_count, &physicalDevice);
    assert(physicalDevice != NULL);

    // 2. Query Displays and Planes
    uint32_t display_count = 0;
    pvkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, &display_count, NULL);
    assert(display_count == 1);
    VkDisplayPropertiesKHR displayProps;
    pvkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, &display_count, &displayProps);
    printf("  [PASS] Found Display: %s (%ux%u)\n", displayProps.displayName, displayProps.physicalResolution.width, displayProps.physicalResolution.height);

    uint32_t plane_count = 0;
    pvkGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, &plane_count, NULL);
    assert(plane_count == 3);
    VkDisplayPlanePropertiesKHR planes[3];
    pvkGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, &plane_count, planes);
    printf("  [PASS] Found %u Hardware Overlay Planes mocked.\n", plane_count);

    // 3. Lease Plane 71 (Index 1) and create surface
    VkSurfaceKHR surface = NULL;
    VkDisplaySurfaceCreateInfoKHR surfaceInfo = {
        .displayMode = (VkDisplayModeKHR)0x1111,
        .planeIndex = 1, // Plane 71
        .planeStackIndex = planes[1].currentStackIndex,
        .transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .globalAlpha = 1.0f,
        .alphaMode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR,
        .imageExtent = {800, 600}
    };
    
    VkResult res = pvkCreateDisplayPlaneSurfaceKHR(instance, &surfaceInfo, NULL, &surface);
    assert(res == VK_SUCCESS && surface != NULL);
    printf("  [PASS] Direct Display Plane Surface created for Plane Index 1 (Hardware Plane 71).\n");

    // 4. Create Device and Swapchain
    VkDevice device = NULL;
    VkDeviceCreateInfo dev_info = {0};
    pvkCreateDevice(physicalDevice, &dev_info, NULL, &device);
    assert(device != NULL);

    VkSwapchainKHR swapchain = NULL;
    VkSwapchainCreateInfoKHR swapInfo = {
        .surface = surface,
        .minImageCount = 2, // Double buffering
        .imageFormat = VK_FORMAT_B8G8R8A8_UNORM,
        .imageExtent = {800, 600},
    };
    res = pvkCreateSwapchainKHR(device, &swapInfo, NULL, &swapchain);
    assert(res == VK_SUCCESS && swapchain != NULL);
    printf("  [PASS] Swapchain configured for double-buffered Page Flipping.\n");

    // 5. Present Frame (Trigger the Page Flip)
    VkQueue queue = NULL;
    pvkGetDeviceQueue(device, 0, 0, &queue);
    
    uint32_t imageIndex = 0;
    VkPresentInfoKHR presentInfo = {
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &imageIndex,
    };
    
    printf("  [ACTION] Queueing Present Request (Submitting DRM Page Flip)...\n");
    res = pvkQueuePresentKHR(queue, &presentInfo);
    assert(res == VK_SUCCESS);
    
    printf("  [PASS] drmModePageFlip execution verified through Firmware ICD.\n");

    // Cleanup
    PFN_vkDestroyDevice pvkDestroyDevice = (PFN_vkDestroyDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyDevice");
    pvkDestroyDevice(device, NULL);
    
    PFN_vkDestroyInstance pvkDestroyInstance = (PFN_vkDestroyInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyInstance");
    pvkDestroyInstance(instance, NULL);
}

int main() {
    alarm(5);
    printf("=== TSFi DRM Display & Page Flip Unit Test ===\n");
    test_firmware_drm_display();
    printf("ALL TESTS PASSED\n");

    extern void tsfi_dl_thunks_teardown(void);
    tsfi_dl_thunks_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
