#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "window_inc/vulkan_struct.h"

extern PFN_vkVoidFunction tsfi_vkGetInstanceProcAddr(VkInstance instance, const char* pName);

int main() {
    alarm(5);
    printf("=== TSFi Vulkan Physical Device Enumeration Test ===\n");

    PFN_vkCreateInstance pvkCreateInstance = (PFN_vkCreateInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    PFN_vkDestroyInstance pvkDestroyInstance = (PFN_vkDestroyInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyInstance");
    PFN_vkEnumeratePhysicalDevices pvkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)tsfi_vkGetInstanceProcAddr(NULL, "vkEnumeratePhysicalDevices");

    // Initialize Instance
    VkInstance instance = NULL;
    VkInstanceCreateInfo inst_info = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    pvkCreateInstance(&inst_info, NULL, &instance);
    assert(instance != NULL);

    // Test 1: Invalid Instance
    uint32_t count = 0;
    VkResult res = pvkEnumeratePhysicalDevices(NULL, &count, NULL);
    assert(res == VK_ERROR_INITIALIZATION_FAILED);
    printf("  [PASS] Rejects NULL VkInstance.\n");

    // Test 2: Invalid Count Pointer
    res = pvkEnumeratePhysicalDevices(instance, NULL, NULL);
    assert(res == VK_ERROR_INITIALIZATION_FAILED);
    printf("  [PASS] Rejects NULL pPhysicalDeviceCount.\n");

    // Test 3: Get Count
    count = 0;
    res = pvkEnumeratePhysicalDevices(instance, &count, NULL);
    assert(res == VK_SUCCESS);
    assert(count == 1);
    printf("  [PASS] Correctly returns physical device count (%u).\n", count);

    // Test 4: Insufficient Array Size
    count = 0;
    VkPhysicalDevice fake_devs[2] = {0};
    res = pvkEnumeratePhysicalDevices(instance, &count, fake_devs);
    assert(res == VK_INCOMPLETE);
    assert(fake_devs[0] == NULL);
    printf("  [PASS] Returns VK_INCOMPLETE when array size is too small.\n");

    // Test 5: Successful retrieval
    count = 1;
    res = pvkEnumeratePhysicalDevices(instance, &count, fake_devs);
    assert(res == VK_SUCCESS);
    assert(fake_devs[0] != NULL);
    printf("  [PASS] Successfully retrieved physical device handles.\n");

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
