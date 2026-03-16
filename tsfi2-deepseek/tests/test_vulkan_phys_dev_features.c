#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "window_inc/vulkan_struct.h"

extern PFN_vkVoidFunction tsfi_vkGetInstanceProcAddr(VkInstance instance, const char* pName);

int main() {
    alarm(5);
    printf("=== TSFi Vulkan Physical Device Features Test ===\n");

    PFN_vkCreateInstance pvkCreateInstance = (PFN_vkCreateInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    PFN_vkDestroyInstance pvkDestroyInstance = (PFN_vkDestroyInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyInstance");
    PFN_vkEnumeratePhysicalDevices pvkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)tsfi_vkGetInstanceProcAddr(NULL, "vkEnumeratePhysicalDevices");
    PFN_vkGetPhysicalDeviceProperties pvkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)tsfi_vkGetInstanceProcAddr(NULL, "vkGetPhysicalDeviceProperties");
    PFN_vkGetPhysicalDeviceMemoryProperties pvkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)tsfi_vkGetInstanceProcAddr(NULL, "vkGetPhysicalDeviceMemoryProperties");
    PFN_vkGetPhysicalDeviceQueueFamilyProperties pvkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)tsfi_vkGetInstanceProcAddr(NULL, "vkGetPhysicalDeviceQueueFamilyProperties");
    PFN_vkEnumerateDeviceExtensionProperties pvkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)tsfi_vkGetInstanceProcAddr(NULL, "vkEnumerateDeviceExtensionProperties");

    VkInstance instance = NULL;
    VkInstanceCreateInfo inst_info = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    pvkCreateInstance(&inst_info, NULL, &instance);
    assert(instance != NULL);

    uint32_t count = 1;
    VkPhysicalDevice dev;
    pvkEnumeratePhysicalDevices(instance, &count, &dev);


        // 1. Test Properties
    VkPhysicalDeviceProperties props;
    pvkGetPhysicalDeviceProperties(dev, &props);
    printf("  [PASS] Device Name: %s\n", props.deviceName);

    printf("  [PASS] Vendor ID: 0x%04X, Device ID: 0x%04X\n", props.vendorID, props.deviceID);

    assert(props.vendorID == 0x1002); // AMD
    assert(props.deviceID == 0x744C); // Navi 48 VCN
    assert(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    assert(props.limits.maxImageDimension1D == 16384);
    assert(props.limits.maxImageDimension2D == 16384);
    assert(props.limits.maxImageDimension3D == 2048);
    assert(props.limits.maxPushConstantsSize == 128); // Critical for PM4 inline constants
    assert(props.limits.maxComputeWorkGroupCount[0] == 65535); // Critical for SVDAG PM4 dispatch
    assert(props.limits.maxComputeWorkGroupCount[1] == 65535);
    assert(props.limits.maxComputeWorkGroupCount[2] == 65535);
    assert(props.limits.maxComputeWorkGroupInvocations == 1024); // Critical for PM4 block sizing
    assert(props.limits.maxComputeWorkGroupSize[0] == 1024);
    assert(props.limits.maxComputeWorkGroupSize[1] == 1024);
    assert(props.limits.maxComputeWorkGroupSize[2] == 64);
    
    printf("  [PASS] Device explicitly supports required TSFi PM4 synthesis compute limitations.\n");


    // 2. Test Memory
    VkPhysicalDeviceMemoryProperties mem;
    pvkGetPhysicalDeviceMemoryProperties(dev, &mem);
    printf("  [PASS] Memory Heaps: %u, Memory Types: %u\n", mem.memoryHeapCount, mem.memoryTypeCount);
    assert(mem.memoryHeapCount == 3);
    assert(mem.memoryTypeCount == 3);
    
    // Check VRAM Heap
    assert(mem.memoryHeaps[0].flags == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
    assert(mem.memoryHeaps[0].size == 16ull * 1024 * 1024 * 1024);
    
    // Check ReBAR Heap
    assert(mem.memoryHeaps[1].flags == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
    assert(mem.memoryHeaps[1].size == 256ull * 1024 * 1024);
    

    // Check pure VRAM memory type
    assert(mem.memoryTypes[0].propertyFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    assert(mem.memoryTypes[0].heapIndex == 0);

    // Check ReBAR memory type
    assert((mem.memoryTypes[1].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0);
    assert((mem.memoryTypes[1].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0);
    assert((mem.memoryTypes[1].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0);
    assert(mem.memoryTypes[1].heapIndex == 1);
    
    // Check GTT Heap & Type
    assert(mem.memoryHeaps[2].flags == 0); // System memory
    assert(mem.memoryHeaps[2].size == 32ull * 1024 * 1024 * 1024);
    assert((mem.memoryTypes[2].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0);
    assert((mem.memoryTypes[2].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0);
    assert((mem.memoryTypes[2].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == 0);
    assert(mem.memoryTypes[2].heapIndex == 2);





    
    // 3. Test Queues
    uint32_t q_count = 0;
    pvkGetPhysicalDeviceQueueFamilyProperties(dev, &q_count, NULL);
    assert(q_count == 5);
    VkQueueFamilyProperties q_props[5];
    pvkGetPhysicalDeviceQueueFamilyProperties(dev, &q_count, q_props);
    
    assert(q_props[0].queueFlags == VK_QUEUE_GRAPHICS_BIT);
    assert(q_props[0].queueCount == 1);
    assert(q_props[1].queueFlags == VK_QUEUE_COMPUTE_BIT);
    assert(q_props[1].queueCount == 15);
    assert(q_props[2].queueFlags == VK_QUEUE_TRANSFER_BIT);
    assert(q_props[2].queueCount == 3);
    assert(q_props[3].queueFlags == VK_QUEUE_VIDEO_DECODE_BIT_KHR);
    assert(q_props[3].queueCount == 1);
    assert(q_props[4].queueFlags == VK_QUEUE_VIDEO_ENCODE_BIT_KHR);
    assert(q_props[4].queueCount == 2);
    
    printf("  [PASS] Queue Families accurately mapped to Hardware IP Rings.\n");


    
    // 4. Test Extensions
    uint32_t ext_count = 0;
    
    // First call: Get count
    pvkEnumerateDeviceExtensionProperties(dev, NULL, &ext_count, NULL);
    assert(ext_count == 4); // Currently we mock exactly 4 extensions
    
    // Second call: Pass smaller count to test bounding and VK_INCOMPLETE
    uint32_t short_count = 2;
    VkExtensionProperties short_exts[2];
    VkResult ext_res = pvkEnumerateDeviceExtensionProperties(dev, NULL, &short_count, short_exts);
    assert(ext_res == VK_INCOMPLETE);
    assert(short_count == 2);
    
    // Third call: Full extraction
    VkExtensionProperties* exts = malloc(sizeof(VkExtensionProperties) * ext_count);
    ext_res = pvkEnumerateDeviceExtensionProperties(dev, NULL, &ext_count, exts);
    assert(ext_res == VK_SUCCESS);
    assert(ext_count == 4);
    
    // Verify exact expected extensions are returned
    int has_swapchain = 0, has_vid_q = 0, has_vid_dec = 0, has_vid_enc = 0;
    for (uint32_t i = 0; i < ext_count; i++) {
        if (strcmp(exts[i].extensionName, "VK_KHR_swapchain") == 0) has_swapchain = 1;
        if (strcmp(exts[i].extensionName, "VK_KHR_video_queue") == 0) has_vid_q = 1;
        if (strcmp(exts[i].extensionName, "VK_KHR_video_decode_queue") == 0) has_vid_dec = 1;
        if (strcmp(exts[i].extensionName, "VK_KHR_video_encode_queue") == 0) has_vid_enc = 1;
    }
    
    assert(has_swapchain);
    assert(has_vid_q);
    assert(has_vid_dec);
    assert(has_vid_enc);

    printf("  [PASS] Successfully retrieved %u required device extensions via proper bounds-checked enumeration.\n", ext_count);

    free(exts);


    pvkDestroyInstance(instance, NULL);

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
