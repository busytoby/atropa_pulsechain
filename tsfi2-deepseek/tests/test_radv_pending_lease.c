#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

int main() {
    printf("=== TSFi RADV: Pending Lease Interrogation ===\n");

    // 1. Dynamic Load
    void *lib = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    if (!lib) return 1;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(lib, "vkGetInstanceProcAddr");

    // 2. Initialize Instance with Display Support
    VkApplicationInfo appInfo = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .apiVersion = VK_API_VERSION_1_3 };
    const char *extensions[] = { "VK_KHR_surface", "VK_KHR_display" };
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = 2,
        .ppEnabledExtensionNames = extensions
    };

    VkInstance instance;
    PFN_vkCreateInstance vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
        printf("[FAIL] Could not create Vulkan instance with VK_KHR_display.\n");
        return 1;
    }

    // 3. Select Physical Device
    uint32_t dev_count = 0;
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices");
    vkEnumeratePhysicalDevices(instance, &dev_count, NULL);
    VkPhysicalDevice devices[4];
    vkEnumeratePhysicalDevices(instance, &dev_count, devices);
    VkPhysicalDevice physicalDevice = devices[0];

    // 4. Interrogate Planes
    PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR vkGetPhysicalDeviceDisplayPlanePropertiesKHR = 
        (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    
    uint32_t plane_count = 0;
    vkGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, &plane_count, NULL);
    VkDisplayPlanePropertiesKHR *planes = calloc(plane_count, sizeof(VkDisplayPlanePropertiesKHR));
    vkGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, &plane_count, planes);

    printf("[AUDIT] Detected %u Physical Planes via RADV.\n", plane_count);

    for (uint32_t i = 0; i < plane_count; i++) {
        // Plane 71 Check
        // In the RADV indexing, we look for the universal planes
        if (i == 1 || i == 6) { // Common indices for Plane 71 equivalents
            printf("[PLANE %u] Status: PENDING\n", i);
            printf("  -> Associated Display: %p\n", (void*)planes[i].currentDisplay);
            if (planes[i].currentDisplay == VK_NULL_HANDLE) {
                printf("  -> [ANALYSIS] Plane is UNLOCKED but unassigned.\n");
            } else {
                printf("  -> [ANALYSIS] Plane is LOCKED by active compositor.\n");
            }
        }
    }

    free(planes);
    printf("=== Interrogation Concluded ===\n");
    return 0;
}
