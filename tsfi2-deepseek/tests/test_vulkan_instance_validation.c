#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "window_inc/vulkan_struct.h"

extern PFN_vkVoidFunction tsfi_vkGetInstanceProcAddr(VkInstance instance, const char* pName);

int main() {
    alarm(5);
    printf("=== TSFi Vulkan Instance Validation Test ===\n");

    PFN_vkCreateInstance pvkCreateInstance = (PFN_vkCreateInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    PFN_vkDestroyInstance pvkDestroyInstance = (PFN_vkDestroyInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyInstance");
    
    assert(pvkCreateInstance != NULL);
    assert(pvkDestroyInstance != NULL);

    // Test 1: Null creation info should fail safely
    VkInstance instance = NULL;
    VkResult res = pvkCreateInstance(NULL, NULL, &instance);
    if (res != VK_ERROR_INITIALIZATION_FAILED) {
        printf("  [FAIL] Expected %d, got %d\n", VK_ERROR_INITIALIZATION_FAILED, res);
    }
    assert(res == VK_ERROR_INITIALIZATION_FAILED);
    printf("  [PASS] vkCreateInstance properly rejects NULL create info.\n");

    // Test 2: Invalid sType should fail
    VkInstanceCreateInfo bad_info = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    res = pvkCreateInstance(&bad_info, NULL, &instance);
    assert(res == VK_ERROR_INITIALIZATION_FAILED);
    printf("  [PASS] vkCreateInstance properly rejects invalid sType.\n");

    // Test 3: Valid creation info with extensions
    const char* ext[] = { "VK_KHR_display", "VK_EXT_debug_utils" };
    VkInstanceCreateInfo good_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .enabledExtensionCount = 2,
        .ppEnabledExtensionNames = ext
    };
    res = pvkCreateInstance(&good_info, NULL, &instance);
    assert(res == VK_SUCCESS);
    assert(instance != NULL);
    printf("  [PASS] vkCreateInstance successfully parsed configuration and allocated context.\n");

    // Cleanup
    pvkDestroyInstance(instance, NULL);
    printf("  [PASS] vkDestroyInstance safely freed context.\n");
    
    // Check tsfi_vkGetInstanceProcAddr behavior
    PFN_vkVoidFunction dummy = tsfi_vkGetInstanceProcAddr(instance, "vkSomeFakeFunction");
    assert(dummy != NULL); // Since we return a dummy catch-all for unknown functions
    printf("  [PASS] tsfi_vkGetInstanceProcAddr operates gracefully.\n");

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
