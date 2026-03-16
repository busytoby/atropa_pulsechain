#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "window_inc/vulkan_struct.h"

extern PFN_vkVoidFunction tsfi_vkGetInstanceProcAddr(VkInstance instance, const char* pName);

int main() {
    alarm(5);
    printf("=== TSFi Vulkan Physical Device Features Evaluation ===\n");

    PFN_vkCreateInstance pvkCreateInstance = (PFN_vkCreateInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    PFN_vkDestroyInstance pvkDestroyInstance = (PFN_vkDestroyInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyInstance");
    PFN_vkEnumeratePhysicalDevices pvkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)tsfi_vkGetInstanceProcAddr(NULL, "vkEnumeratePhysicalDevices");
    PFN_vkGetPhysicalDeviceFeatures pvkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures)tsfi_vkGetInstanceProcAddr(NULL, "vkGetPhysicalDeviceFeatures");
    PFN_vkGetPhysicalDeviceFeatures2 pvkGetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2)tsfi_vkGetInstanceProcAddr(NULL, "vkGetPhysicalDeviceFeatures2");

    VkInstance instance = NULL;
    VkInstanceCreateInfo inst_info = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    pvkCreateInstance(&inst_info, NULL, &instance);
    assert(instance != NULL);

    uint32_t count = 1;
    VkPhysicalDevice dev;
    pvkEnumeratePhysicalDevices(instance, &count, &dev);

    // Test 1: GetPhysicalDeviceFeatures
    VkPhysicalDeviceFeatures features;
    pvkGetPhysicalDeviceFeatures(dev, &features);
    
    assert(features.geometryShader == VK_TRUE);
    assert(features.tessellationShader == VK_TRUE);
    assert(features.multiDrawIndirect == VK_TRUE);
    assert(features.shaderFloat64 == VK_TRUE);
    assert(features.shaderInt64 == VK_TRUE);
    assert(features.sparseBinding == VK_TRUE);
    
    printf("  [PASS] vkGetPhysicalDeviceFeatures reports valid high-end AMD features.\n");

    // Test 2: GetPhysicalDeviceFeatures2
    VkPhysicalDeviceFeatures2 features2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
    pvkGetPhysicalDeviceFeatures2(dev, &features2);
    
    assert(features2.features.geometryShader == VK_TRUE);
    assert(features2.features.tessellationShader == VK_TRUE);
    assert(features2.features.multiDrawIndirect == VK_TRUE);
    assert(features2.features.shaderFloat64 == VK_TRUE);
    assert(features2.features.shaderInt64 == VK_TRUE);
    assert(features2.features.sparseBinding == VK_TRUE);

    printf("  [PASS] vkGetPhysicalDeviceFeatures2 safely embeds and populates core features.\n");

    // Test 3: GetPhysicalDeviceFeatures2 with pNext chaining
    typedef struct tsfi_VkPhysicalDeviceVulkan12Features {
        VkStructureType    sType;
        void*              pNext;
        VkBool32           samplerMirrorClampToEdge;
        VkBool32           drawIndirectCount;
        VkBool32           storageBuffer8BitAccess;
        VkBool32           uniformAndStorageBuffer8BitAccess;
        VkBool32           storagePushConstant8;
        VkBool32           shaderBufferInt64Atomics;
        VkBool32           shaderSharedInt64Atomics;
        VkBool32           shaderFloat16;
        VkBool32           shaderInt8;
        VkBool32           descriptorIndexing;
        VkBool32           shaderInputAttachmentArrayDynamicIndexing;
        VkBool32           shaderUniformTexelBufferArrayDynamicIndexing;
        VkBool32           shaderStorageTexelBufferArrayDynamicIndexing;
        VkBool32           shaderUniformBufferArrayNonUniformIndexing;
        VkBool32           shaderSampledImageArrayNonUniformIndexing;
        VkBool32           shaderStorageBufferArrayNonUniformIndexing;
        VkBool32           shaderStorageImageArrayNonUniformIndexing;
        VkBool32           shaderInputAttachmentArrayNonUniformIndexing;
        VkBool32           shaderUniformTexelBufferArrayNonUniformIndexing;
        VkBool32           shaderStorageTexelBufferArrayNonUniformIndexing;
        VkBool32           descriptorBindingUniformBufferUpdateAfterBind;
        VkBool32           descriptorBindingSampledImageUpdateAfterBind;
        VkBool32           descriptorBindingStorageImageUpdateAfterBind;
        VkBool32           descriptorBindingStorageBufferUpdateAfterBind;
        VkBool32           descriptorBindingUniformTexelBufferUpdateAfterBind;
        VkBool32           descriptorBindingStorageTexelBufferUpdateAfterBind;
        VkBool32           descriptorBindingUpdateUnusedWhilePending;
        VkBool32           descriptorBindingPartiallyBound;
        VkBool32           descriptorBindingVariableDescriptorCount;
        VkBool32           runtimeDescriptorArray;
        VkBool32           samplerFilterMinmax;
        VkBool32           scalarBlockLayout;
        VkBool32           imagelessFramebuffer;
        VkBool32           uniformBufferStandardLayout;
        VkBool32           shaderSubgroupExtendedTypes;
        VkBool32           separateDepthStencilLayouts;
        VkBool32           hostQueryReset;
        VkBool32           timelineSemaphore;
        VkBool32           bufferDeviceAddress;
        VkBool32           bufferDeviceAddressCaptureReplay;
        VkBool32           bufferDeviceAddressMultiDevice;
        VkBool32           vulkanMemoryModel;
        VkBool32           vulkanMemoryModelDeviceScope;
        VkBool32           vulkanMemoryModelAvailabilityVisibilityChains;
        VkBool32           shaderOutputViewportIndex;
        VkBool32           shaderOutputLayer;
        VkBool32           subgroupBroadcastDynamicId;
    } tsfi_VkPhysicalDeviceVulkan12Features;

    tsfi_VkPhysicalDeviceVulkan12Features v12_features = { .sType = 51 }; // VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES
    VkPhysicalDeviceFeatures2 features2_chain = { 
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &v12_features
    };
    
    pvkGetPhysicalDeviceFeatures2(dev, &features2_chain);
    
    assert(v12_features.bufferDeviceAddress == VK_TRUE);
    assert(v12_features.timelineSemaphore == VK_TRUE);
    assert(v12_features.descriptorIndexing == VK_TRUE);

    printf("  [PASS] vkGetPhysicalDeviceFeatures2 safely traverses pNext and populates Vulkan 1.2 structs.\n");


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
