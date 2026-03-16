#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "lau_memory.h"
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include <vulkan/vulkan.h>

extern void* tsfi_vkGetInstanceProcAddr(VkInstance instance, const char* pName);
extern LauMetadata* lau_registry_find(void *payload);

int main() {
    printf("=== TSFi Vulkan Device Queue Exhaustive Audit ===\n");
    alarm(5);
    
    // 1. Initialize the global hardware manifold
    tsfi_wire_firmware_init();
    
    // 2. Load the Vulkan ICD pointers using our reflection engine
    PFN_vkCreateInstance pvkCreateInstance = (PFN_vkCreateInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    PFN_vkCreateDevice pvkCreateDevice = (PFN_vkCreateDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateDevice");
    PFN_vkGetDeviceQueue pvkGetDeviceQueue = (PFN_vkGetDeviceQueue)tsfi_vkGetInstanceProcAddr(NULL, "vkGetDeviceQueue");
    PFN_vkQueueSubmit pvkQueueSubmit = (PFN_vkQueueSubmit)tsfi_vkGetInstanceProcAddr(NULL, "vkQueueSubmit");
    PFN_vkQueueWaitIdle pvkQueueWaitIdle = (PFN_vkQueueWaitIdle)tsfi_vkGetInstanceProcAddr(NULL, "vkQueueWaitIdle");
    PFN_vkDestroyDevice pvkDestroyDevice = (PFN_vkDestroyDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyDevice");
    PFN_vkDestroyInstance pvkDestroyInstance = (PFN_vkDestroyInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyInstance");

    assert(pvkCreateInstance && pvkCreateDevice && pvkGetDeviceQueue && pvkQueueSubmit && pvkQueueWaitIdle && pvkDestroyDevice);

    VkInstance instance = VK_NULL_HANDLE;
    VkInstanceCreateInfo instInfo = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    pvkCreateInstance(&instInfo, NULL, &instance);

    VkDevice device = VK_NULL_HANDLE;
    VkDeviceCreateInfo devInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    pvkCreateDevice(VK_NULL_HANDLE, &devInfo, NULL, &device);
    
    assert(device != VK_NULL_HANDLE);
    
    // 3. Request the hardware queue
    VkQueue queue = VK_NULL_HANDLE;
    uint32_t target_family = 1;
    uint32_t target_index = 0;
    
    printf("[TEST] Invoking vkGetDeviceQueue(family=%u, index=%u)...\n", target_family, target_index);
    pvkGetDeviceQueue(device, target_family, target_index, &queue);
    
    assert(queue != VK_NULL_HANDLE);
    printf("[TEST] Received queue handle: %p\n", (void*)queue);

    // 4. Verify Thunk Proxy Schema Injection (If the queue is a real TSFiQueue, it has a wired header and proxy)
    // We check if it is backed by our memory allocator
    LauMetadata* meta = lau_registry_find((void*)queue);
    
    if (meta) {
        printf("[TEST] Queue is a physically managed hardware object (Size: %zu).\n", meta->alloc_size & 0x007FFFFFFFFFFFFFULL);
        uint8_t type = (uint8_t)(meta->alloc_size >> 56);
        assert(type == LAU_TYPE_WIRED && "Queue must be a WIRED hardware object.");
        
        // Ensure it was sealed
        assert((meta->alloc_size & (1ULL << 55)) && "Queue MUST be physically sealed upon creation.");
        printf("[TEST] Physical Seal Verified.\n");
    } else {
        printf("[TEST] WARNING: Queue is a static mock or untracked pointer!\n");
    }

    // 5. Submit Work
    printf("[TEST] Submitting mock workload...\n");
    VkSubmitInfo submitInfo = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO };
    VkResult submit_res = pvkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    assert(submit_res == VK_SUCCESS);
    
    // 6. Wait Idle
    printf("[TEST] Waiting for queue idle...\n");
    VkResult idle_res = pvkQueueWaitIdle(queue);
    assert(idle_res == VK_SUCCESS);

    // 7. Cleanup
    printf("[TEST] Destroying device...\n");
    pvkDestroyDevice(device, NULL);
    pvkDestroyInstance(instance, NULL);
    
    extern void lau_free_all_active(void);
    lau_free_all_active();

    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_vulkan_device_queue", NULL);

    printf("=== VULKAN QUEUE AUDIT PASSED ===\n");

    return 0;
}
