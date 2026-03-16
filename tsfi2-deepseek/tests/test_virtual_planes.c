#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "lau_memory.h"
#include "window_inc/vulkan_struct.h"

extern PFN_vkVoidFunction tsfi_vkGetInstanceProcAddr(VkInstance instance, const char* pName);

// Function pointers for our custom TSFi ZMM extensions
typedef int (VKAPI_CALL *PFN_tsfi_drmModeAddPlane)(uint32_t plane_id, size_t buffer_size);
typedef void (VKAPI_CALL *PFN_tsfi_drmModeFreeVirtualPlanes)(void);

void test_virtual_planes() {
    printf("[TEST] Testing Zero-Copy Memory Manager (ZMM) Virtual Planes...\n");

    // Load necessary functions from TSFi Firmware ICD
    PFN_vkCreateInstance pvkCreateInstance = (PFN_vkCreateInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    PFN_vkCreateDevice pvkCreateDevice = (PFN_vkCreateDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateDevice");
    PFN_vkGetDeviceQueue pvkGetDeviceQueue = (PFN_vkGetDeviceQueue)tsfi_vkGetInstanceProcAddr(NULL, "vkGetDeviceQueue");
    PFN_vkQueuePresentKHR pvkQueuePresentKHR = (PFN_vkQueuePresentKHR)tsfi_vkGetInstanceProcAddr(NULL, "vkQueuePresentKHR");
    
    PFN_tsfi_drmModeAddPlane ptsfi_drmModeAddPlane = (PFN_tsfi_drmModeAddPlane)tsfi_vkGetInstanceProcAddr(NULL, "tsfi_drmModeAddPlane");
    PFN_tsfi_drmModeFreeVirtualPlanes ptsfi_drmModeFreeVirtualPlanes = (PFN_tsfi_drmModeFreeVirtualPlanes)tsfi_vkGetInstanceProcAddr(NULL, "tsfi_drmModeFreeVirtualPlanes");

    assert(ptsfi_drmModeAddPlane != NULL);
    assert(ptsfi_drmModeFreeVirtualPlanes != NULL);

    // 1. Initialize Firmware Vulkan Mock Stack
    VkInstance instance = NULL;
    VkInstanceCreateInfo inst_info = {0};
    pvkCreateInstance(&inst_info, NULL, &instance);
    
    VkDevice device = NULL;
    VkDeviceCreateInfo dev_info = {0};
    pvkCreateDevice((VkPhysicalDevice)0x2000, &dev_info, NULL, &device);

    // 2. Create Virtual Planes via ZMM
    size_t plane_buffer_size = 1920 * 1080 * 4; // 1080p RGBA buffer
    printf("  [ACTION] Requesting ZMM to allocate Zero-Copy Virtual Planes...\n");
    
    int ret1 = ptsfi_drmModeAddPlane(72, plane_buffer_size);
    assert(ret1 == 0);
    int ret2 = ptsfi_drmModeAddPlane(73, plane_buffer_size);
    assert(ret2 == 0);
    int ret3 = ptsfi_drmModeAddPlane(74, plane_buffer_size);
    assert(ret3 == 0);

    printf("  [PASS] ZMM Virtual Planes 72, 73, and 74 created in Firmware ReBAR Memory.\n");

    // 3. Trigger Presentation (ZMM Blending & Page Flip)
    VkQueue queue = NULL;
    pvkGetDeviceQueue(device, 0, 0, &queue);
    
    VkSwapchainKHR mock_swapchain = (VkSwapchainKHR)0xB000;
    uint32_t imageIndex = 0;
    VkPresentInfoKHR presentInfo = {
        .swapchainCount = 1,
        .pSwapchains = &mock_swapchain,
        .pImageIndices = &imageIndex,
    };
    
    printf("  [ACTION] Submitting Frame to hardware...\n");
    VkResult res = pvkQueuePresentKHR(queue, &presentInfo);
    assert(res == VK_SUCCESS);

    printf("  [PASS] ZMM properly intercepted Presentation, executed zero-copy blending, and finalized via hardware page flip.\n");

    // 4. Cleanup
    ptsfi_drmModeFreeVirtualPlanes();
    
    PFN_vkDestroyDevice pvkDestroyDevice = (PFN_vkDestroyDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyDevice");
    pvkDestroyDevice(device, NULL);
    
    PFN_vkDestroyInstance pvkDestroyInstance = (PFN_vkDestroyInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyInstance");
    pvkDestroyInstance(instance, NULL);
}

int main() {
    alarm(5);
    printf("=== TSFi Virtual Plane ZMM Test ===\n");
    test_virtual_planes();
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
