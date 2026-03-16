#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "window_inc/vulkan_struct.h"

extern PFN_vkVoidFunction tsfi_vkGetInstanceProcAddr(VkInstance instance, const char* pName);

int main() {
    alarm(5);
    printf("=== TSFi Vulkan Queue Family Properties Test ===\n");

    PFN_vkCreateInstance pvkCreateInstance = (PFN_vkCreateInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    PFN_vkDestroyInstance pvkDestroyInstance = (PFN_vkDestroyInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyInstance");
    PFN_vkEnumeratePhysicalDevices pvkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)tsfi_vkGetInstanceProcAddr(NULL, "vkEnumeratePhysicalDevices");
    PFN_vkGetPhysicalDeviceQueueFamilyProperties pvkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)tsfi_vkGetInstanceProcAddr(NULL, "vkGetPhysicalDeviceQueueFamilyProperties");

    VkInstance instance = NULL;
    VkInstanceCreateInfo inst_info = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    pvkCreateInstance(&inst_info, NULL, &instance);
    assert(instance != NULL);

    uint32_t count = 1;
    VkPhysicalDevice dev;
    pvkEnumeratePhysicalDevices(instance, &count, &dev);

    // Test 1: Querying the number of queue families
    uint32_t q_count = 0;
    pvkGetPhysicalDeviceQueueFamilyProperties(dev, &q_count, NULL);
    assert(q_count == 5);
    printf("  [PASS] Correctly reports exactly 5 distinct hardware queue families.\n");

    // Test 2: Passing a smaller count bounds the write but doesn't return VK_INCOMPLETE
    // Vulkan specification states that if pQueueFamilyProperties is not NULL,
    // the number of structures written is the minimum of the available count and the passed count.
    uint32_t short_count = 0;
    VkQueueFamilyProperties dummy_prop[5];
    // We pass 0, so it should not write anything.
    pvkGetPhysicalDeviceQueueFamilyProperties(dev, &short_count, dummy_prop);
    assert(short_count == 0);
    printf("  [PASS] Correctly handles bounded writes (writing 0 elements).\n");

    // Test 3: Fetching the actual properties
    VkQueueFamilyProperties q_props[5];
    pvkGetPhysicalDeviceQueueFamilyProperties(dev, &q_count, q_props);
    

    // Assert the exact hardware rings (families)
    assert(q_props[0].queueFlags == VK_QUEUE_GRAPHICS_BIT);
    assert(q_props[0].queueCount == 1); // 1 GFX Ring

    assert(q_props[1].queueFlags == VK_QUEUE_COMPUTE_BIT);
    assert(q_props[1].queueCount == 15); // 15 Compute Rings

    assert(q_props[2].queueFlags == VK_QUEUE_TRANSFER_BIT);
    assert(q_props[2].queueCount == 3); // 3 DMA Rings

    assert(q_props[3].queueFlags == VK_QUEUE_VIDEO_DECODE_BIT_KHR);
    assert(q_props[3].queueCount == 1); // 1 UVD (Decode) Ring

    assert(q_props[4].queueFlags == VK_QUEUE_VIDEO_ENCODE_BIT_KHR);
    assert(q_props[4].queueCount == 2); // 1 VCE + 1 UVD_ENC (Encode) Rings

    printf("  [PASS] Queue Families accurately mapped to %u Hardware IP Rings.\n", q_count);


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
