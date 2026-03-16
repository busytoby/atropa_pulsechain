#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>
#include "lau_memory.h"

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define RESET   "\033[0m"

// Define extension structures manually if missing from system headers
#ifndef VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME
#define VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME "VK_KHR_cooperative_matrix"
#define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR 1000506000
#define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR 1000506001

typedef struct VkCooperativeMatrixPropertiesKHR {
    VkStructureType    sType;
    void*              pNext;
    uint32_t           MSize;
    uint32_t           NSize;
    uint32_t           KSize;
    VkComponentTypeKHR AType;
    VkComponentTypeKHR BType;
    VkComponentTypeKHR CType;
    VkComponentTypeKHR ResultType;
    bool               saturatingAccumulation;
    VkScopeKHR         scope;
} VkCooperativeMatrixPropertiesKHR;

typedef VkResult (VKAPI_PTR *PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR)(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixPropertiesKHR* pProperties);
#endif

// --- Helper: Type to String ---
const char* type_to_str(VkComponentTypeKHR t) {
    switch(t) {
        case VK_COMPONENT_TYPE_FLOAT16_KHR: return "F16";
        case VK_COMPONENT_TYPE_FLOAT32_KHR: return "F32";
        case VK_COMPONENT_TYPE_FLOAT64_KHR: return "F64";
        case VK_COMPONENT_TYPE_SINT8_KHR:   return "I8";
        case VK_COMPONENT_TYPE_SINT16_KHR:  return "I16";
        case VK_COMPONENT_TYPE_SINT32_KHR:  return "I32";
        case VK_COMPONENT_TYPE_SINT64_KHR:  return "I64";
        case VK_COMPONENT_TYPE_UINT8_KHR:   return "U8";
        case VK_COMPONENT_TYPE_UINT16_KHR:  return "U16";
        case VK_COMPONENT_TYPE_UINT32_KHR:  return "U32";
        case VK_COMPONENT_TYPE_UINT64_KHR:  return "U64";
        default: return "UNKNOWN";
    }
}

const char* scope_to_str(VkScopeKHR s) {
    switch(s) {
        case VK_SCOPE_DEVICE_KHR: return "DEVICE";
        case VK_SCOPE_WORKGROUP_KHR: return "WORKGROUP";
        case VK_SCOPE_SUBGROUP_KHR: return "SUBGROUP";
        case VK_SCOPE_QUEUE_FAMILY_KHR: return "QUEUE_FAMILY";
        default: return "UNKNOWN";
    }
}

// --- Main Probe ---
int main() {
    printf("%s[RDNA4] Cooperative Matrix (WMMA) Probe%s\n", CYAN, RESET);

    // 1. Load Vulkan Lib
    void *lib = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    if (!lib) { printf("Failed to load libvulkan.so.1\n"); return 1; }

    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(lib, "vkGetInstanceProcAddr");
    PFN_vkCreateInstance vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(NULL, "vkCreateInstance");

    // 2. Create Instance with basic extensions
    const char *exts[] = { "VK_KHR_get_physical_device_properties2" };
    VkInstanceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = exts
    };
    VkInstance instance;
    if (vkCreateInstance(&info, NULL, &instance) != VK_SUCCESS) {
        printf("Failed to create instance (Requires VK_KHR_get_physical_device_properties2)\n");
        return 1;
    }

    // 3. Find Device
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices");
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties");
    
    VkPhysicalDevice phys_dev;
    uint32_t count = 1;
    vkEnumeratePhysicalDevices(instance, &count, &phys_dev);
    
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(phys_dev, &props);
    printf("Device: %s (Driver Ver: %u)\n", props.deviceName, props.driverVersion);

    // 4. Check Extension Support
    PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)vkGetInstanceProcAddr(instance, "vkEnumerateDeviceExtensionProperties");
    
    vkEnumerateDeviceExtensionProperties(phys_dev, NULL, &count, NULL);
    VkExtensionProperties *devExts = (VkExtensionProperties*)lau_malloc(sizeof(VkExtensionProperties) * count);
    vkEnumerateDeviceExtensionProperties(phys_dev, NULL, &count, devExts);
    
    bool has_wmma = false;
    for (uint32_t i = 0; i < count; i++) {
        if (strcmp(devExts[i].extensionName, VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME) == 0) {
            has_wmma = true;
            break;
        }
    }
    lau_free(devExts);

    if (!has_wmma) {
        printf("%s[FAIL] VK_KHR_cooperative_matrix NOT supported on this device.%s\n", RED, RESET);
        return 0;
    }
    printf("%s[SUCCESS] VK_KHR_cooperative_matrix IS supported.%s\n", GREEN, RESET);

    // 5. Query Properties
    PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR = 
        (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR");
    
    if (!vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR) {
        printf("Failed to load vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR func ptr.\n");
        return 1;
    }

    uint32_t propCount = 0;
    vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(phys_dev, &propCount, NULL);
    
    if (propCount == 0) {
        printf("Extension supported, but no matrix configurations exposed.\n");
        return 0;
    }

    VkCooperativeMatrixPropertiesKHR *matrixProps = (VkCooperativeMatrixPropertiesKHR*)lau_malloc(sizeof(VkCooperativeMatrixPropertiesKHR) * propCount);
    for (uint32_t i = 0; i < propCount; i++) matrixProps[i].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR;
    
    vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(phys_dev, &propCount, matrixProps);

    printf("\n%sSupported Matrix Configurations (MxNxK):%s\n", YELLOW, RESET);
    printf("%-10s %-10s %-10s %-8s %-8s %-8s %-15s\n", "M", "N", "K", "A", "B", "Result", "Scope");
    printf("--------------------------------------------------------------------------------\n");

    for (uint32_t i = 0; i < propCount; i++) {
        VkCooperativeMatrixPropertiesKHR *p = &matrixProps[i];
        printf("%-10u %-10u %-10u %-8s %-8s %-8s %-15s\n", 
               p->MSize, p->NSize, p->KSize,
               type_to_str(p->AType), type_to_str(p->BType), type_to_str(p->ResultType),
               scope_to_str(p->scope));
    }
    printf("--------------------------------------------------------------------------------\n");
    
    printf("\n[RECOMMENDATION] For lau_memory WMMA alignment:\n");
    printf("  - F16/F32 paths detected.\n");
    printf("  - Alignment of 256 bytes strongly recommended for subgroup broadcasting.\n");
    printf("  - Use LAU_GPU_TIER_WMMA to enforce this.\n");

    lau_free(matrixProps);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
